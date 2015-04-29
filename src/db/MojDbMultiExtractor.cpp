#include "db/MojDbMultiExtractor.h"
#include "db/MojDbPropExtractor.h"

const MojChar* const MojDbMultiExtractor::IncludeKey = _T("include");


MojErr MojDbMultiExtractor::fromObject(const MojObject& obj, const MojChar* locale)
{
	MojErr err = MojDbExtractor::fromObject(obj, locale);
	MojErrCheck(err);

	// load the default config
	MojDbPropExtractor defaultExtractor;
	err = defaultExtractor.fromObject(obj, locale);
	MojErrCheck(err);

	MojObject include;
	err = obj.getRequired(IncludeKey, include);
	MojErrCheck(err);

	for (MojObject::ConstArrayIterator i = include.arrayBegin();
		 i != include.arrayEnd(); ++i) {
		MojString includeName;
	err = i->stringValue(includeName);
	MojErrCheck(err);
	MojRefCountedPtr<MojDbPropExtractor> propExtractor(new MojDbPropExtractor);
	MojAllocCheck(propExtractor.get());
	err = propExtractor->fromObjectImpl(*i, defaultExtractor, locale);
	MojErrCheck(err);
	err = m_extractors.push(propExtractor);
	MojErrCheck(err);
		 }
		 return MojErrNone;
}

MojErr MojDbMultiExtractor::updateLocale(const MojChar* locale)
{
	for (ExtractorVec::ConstIterator i = m_extractors.begin(); i != m_extractors.end(); ++i) {
		MojErr err = (*i)->updateLocale(locale);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbMultiExtractor::vals(const MojObject& obj, KeySet& valsOut) const
{
	// extract property values
	for (ExtractorVec::ConstIterator i = m_extractors.begin(); i != m_extractors.end(); ++i) {
		MojErr err = (*i)->vals(obj, valsOut);
		MojErrCheck(err);
	}
	return MojErrNone;
}
