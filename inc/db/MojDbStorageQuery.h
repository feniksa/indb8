#pragma once

#include "core/MojRefCount.h"
#include "db/MojDbDefs.h"
#include "db/MojDbQuery.h"

class MojDbStorageQuery : public MojRefCounted
{
public:
	typedef MojVector<MojByte> ByteVec;
	typedef MojSet<MojString> StringSet;

	MojDbStorageQuery() {}
	virtual ~MojDbStorageQuery() {}
	virtual MojErr close() = 0;
	virtual MojErr get(MojDbStorageItem*& itemOut, bool& foundOut) = 0;
	virtual MojErr getId(MojObject& idOut, MojUInt32& groupOut, bool& foundOut) = 0;
	virtual MojErr getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut) = 0;
	virtual MojErr count(MojUInt32& countOut) = 0;
	virtual MojErr nextPage(MojDbQuery::Page& pageOut) = 0;
	virtual void excludeKinds(const StringSet& toExclude) { m_excludeKinds = toExclude; }
	virtual MojUInt32 groupCount() const = 0;
	const MojDbKey& endKey() const { return m_endKey; }
	StringSet& excludeKinds() { return m_excludeKinds; }
	bool verify() { return m_verify; }
	void verify(bool bVal) { m_verify = bVal; }

protected:
	MojDbKey m_endKey;
	StringSet m_excludeKinds;
	bool m_verify;

};
