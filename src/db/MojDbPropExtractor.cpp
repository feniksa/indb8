#include "db/MojDbPropExtractor.h"

const MojChar MojDbPropExtractor::PropComponentSeparator = _T('.');
const MojChar* const MojDbPropExtractor::AllKey = _T("all");
const MojChar* const MojDbPropExtractor::DefaultKey = _T("default");
const MojChar* const MojDbPropExtractor::PrimaryKey = _T("primary");
const MojChar* const MojDbPropExtractor::SecondaryKey = _T("secondary");
const MojChar* const MojDbPropExtractor::TertiaryKey = _T("tertiary");
const MojChar* const MojDbPropExtractor::TokenizeKey = _T("tokenize");
const MojChar* const MojDbPropExtractor::WildcardKey = _T("*");

MojDbPropExtractor::MojDbPropExtractor()
{
}

MojErr MojDbPropExtractor::prop(const MojString& name)
{
	// split prop name into components
	MojErr err = name.split(PropComponentSeparator, m_prop);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPropExtractor::fromObject(const MojObject& obj, const MojChar* locale)
{
	MojDbPropExtractor defaultConfig;
	MojErr err = fromObjectImpl(obj, defaultConfig, locale);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPropExtractor::updateLocale(const MojChar* locale)
{
	if (m_collation != MojDbCollationInvalid) {
		if(m_tokenizer.get())
		{
			m_tokenizer.reset(new MojDbTextTokenizer());
			MojAllocCheck(m_tokenizer.get());
			MojErr err = m_tokenizer->init(locale);
			MojErrCheck(err);
		}

		m_collator.reset(new MojDbTextCollator());
		MojAllocCheck(m_collator.get());
		MojErr err = m_collator->init(locale, m_collation);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbPropExtractor::fromObjectImpl(const MojObject& obj, const MojDbPropExtractor& defaultConfig, const MojChar* locale)
{
	// super
	MojErr err = MojDbExtractor::fromObject(obj, locale);
	MojErrCheck(err);

	// default value
	m_default = defaultConfig.m_default;
	MojObject defaultVal;
	if (obj.get(DefaultKey, defaultVal)) {
		MojObject::Type type = defaultVal.type();
		MojDbKey key;
		if (type == MojObject::TypeArray) {
			// if the value is an array, act on its elements rather than the array object itself
			MojObject::ConstArrayIterator end = defaultVal.arrayEnd();
			for (MojObject::ConstArrayIterator j = defaultVal.arrayBegin(); j != end; ++j) {
				err = key.assign(*j);
				MojErrCheck(err);
				err = m_default.put(key);
				MojErrCheck(err);
			}
		} else {
			err = key.assign(defaultVal);
			MojErrCheck(err);
			err = m_default.put(key);
			MojErrCheck(err);
		}
	}

	// tokenizer
	m_tokenizer = defaultConfig.m_tokenizer;
	MojString tokenize;
	bool found = false;
	err = obj.get(TokenizeKey, tokenize, found);
	MojErrCheck(err);
	if (found) {
		if (tokenize == AllKey || tokenize == DefaultKey) {
			m_tokenizer.reset(new MojDbTextTokenizer);
			MojAllocCheck(m_tokenizer.get());
			err = m_tokenizer->init(locale);
			MojErrCheck(err);
		} else {
			MojErrThrow(MojErrDbInvalidTokenization);
		}
	}

	// collator
	if (m_collation == MojDbCollationInvalid)
		m_collation = defaultConfig.m_collation;
	err = updateLocale(locale);
	MojErrCheck(err);

	// set prop
	err = prop(m_name);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPropExtractor::valsImpl(const MojObject& obj, KeySet& valsOut, MojSize idx) const
{
	MojAssert(!m_prop.empty() && idx < m_prop.size());

	MojErr err = MojErrNone;
	const MojString& propKey = m_prop[idx];
	if (propKey == WildcardKey) {
		// get all prop vals if we have a wildcard
		for (MojObject::ConstIterator i = obj.begin(); i != obj.end(); ++i) {
			err = handleVal(*i, valsOut, idx);
			MojErrCheck(err);
		}
	} else {
		// get object corresponding to the current component in the prop path
		MojObject::ConstIterator i = obj.find(propKey);
		if (i == obj.end()) {
			err = valsOut.put(m_default);
			MojErrCheck(err);
		} else {
			if (i->type() == MojObject::TypeArray) {
				// if the value is an array, act on its elements rather than the array object itself
				MojObject::ConstArrayIterator end = i->arrayEnd();
				for (MojObject::ConstArrayIterator j = i->arrayBegin(); j != end; ++j) {
					err = handleVal(*j, valsOut, idx);
					MojErrCheck(err);
				}
			} else {
				// not an array
				err = handleVal(*i, valsOut, idx);
				MojErrCheck(err);
			}
		}
	}
	return MojErrNone;
}

MojErr MojDbPropExtractor::handleVal(const MojObject& val, KeySet& valsOut, MojSize idx) const
{
	MojAssert(idx < m_prop.size());

	MojErr err = MojErrNone;
	if (idx == m_prop.size() - 1) {
		// if we're at the end of the prop path, use this object as the value
		if (m_tokenizer.get() && val.type() == MojObject::TypeString) {
			MojString text;
			err = val.stringValue(text);
			MojErrCheck(err);
			if (m_tokenizer.get()) {
				err = m_tokenizer->tokenize(text, m_collator.get(), valsOut);
				MojErrCheck(err);
			}
		} else {
			MojDbKey key;
			err = key.assign(val, m_collator.get());
			MojErrCheck(err);
			err = valsOut.put(key);
			MojErrCheck(err);
		}
	} else {
		// otherwise, keep recursing
		err = valsImpl(val, valsOut, idx + 1);
		MojErrCheck(err);
	}
	return MojErrNone;
}