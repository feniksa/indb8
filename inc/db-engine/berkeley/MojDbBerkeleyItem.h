#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojFile.h"
#include "core/MojBuffer.h"
#include "db/MojDbStorageItem.h"
#include "db/MojDbObjectHeader.h"
#include <db.h>

class MojDbBerkeleyItem : public MojDbStorageItem
{
public:
	MojDbBerkeleyItem();
	virtual ~MojDbBerkeleyItem() { freeData(); }
	virtual MojErr close() { return MojErrNone; }
	virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine);
	virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
	virtual const MojObject& id() const { return m_header.id(); }
	virtual MojSize size() const { return m_dbt.size; }

	void clear();
	const MojByte* data() const { return (const MojByte*) m_dbt.data; }
	bool hasPrefix(const MojDbKey& prefix) const;
	MojErr toArray(MojObject& arrayOut) const;
	MojErr toObject(MojObject& objOut) const;

	void id(const MojObject& id);
	void fromBytesNoCopy(const MojByte* bytes, MojSize size);
	MojErr fromBuffer(MojBuffer& buf);
	MojErr fromBytes(const MojByte* bytes, MojSize size);
	MojErr fromObject(const MojObject& obj);
	MojErr fromObjectVector(const MojVector<MojObject>& vec);

	DBT* impl() { return &m_dbt; }

private:
	void freeData();
	void setData(MojByte* bytes, MojSize size, void (*free)(void*));

	DBT m_dbt;
	MojAutoPtr<MojBuffer::Chunk> m_chunk;
	mutable MojDbObjectHeader m_header;
	void (*m_free)(void*);
};
