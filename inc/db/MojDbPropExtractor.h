#pragma once

#include "db/MojDbExtractor.h"

class MojDbPropExtractor : public MojDbExtractor
{
public:
	static const MojChar* const AllKey;
	static const MojChar* const DefaultKey;
	static const MojChar* const PrimaryKey;
	static const MojChar* const SecondaryKey;
	static const MojChar* const TertiaryKey;
	static const MojChar* const TokenizeKey;

	MojDbPropExtractor();
	void collator(MojDbTextCollator* collator) { m_collator.reset(collator); }
	MojErr prop(const MojString& name);

	MojErr fromObject(const MojObject& obj, const MojChar* locale) override;
	MojErr updateLocale(const MojChar* locale) override;
	MojErr vals(const MojObject& obj, KeySet& valsOut) const override { return valsImpl(obj, valsOut, 0); }

private:
	friend class MojDbMultiExtractor;
	using StringVec = MojVector<MojString>;

	static const MojChar PropComponentSeparator;
	static const MojChar* const WildcardKey;

	MojErr fromObjectImpl(const MojObject& obj, const MojDbPropExtractor& defaultConfig, const MojChar* locale);
	MojErr valsImpl(const MojObject& obj, KeySet& valsOut, MojSize idx) const;
	MojErr handleVal(const MojObject& val, KeySet& valsOut, MojSize idx) const;

	KeySet m_default;
	StringVec m_prop;
	MojRefCountedPtr<MojDbTextTokenizer> m_tokenizer;
	MojRefCountedPtr<MojDbTextCollator> m_collator;
};
