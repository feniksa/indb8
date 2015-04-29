#include "db/MojDbExtractor.h"

class MojDbMultiExtractor : public MojDbExtractor
{
public:
	static const MojChar* const IncludeKey; // include

	MojErr fromObject(const MojObject& obj, const MojChar* locale) override;
	MojErr updateLocale(const MojChar* locale) override;
	MojErr vals(const MojObject& obj, KeySet& valsOut) const override;

private:
	typedef MojVector<MojRefCountedPtr<MojDbExtractor> > ExtractorVec;

	ExtractorVec m_extractors;
};
