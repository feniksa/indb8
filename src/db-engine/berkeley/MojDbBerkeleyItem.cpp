#include "db-engine/berkeley/MojDbBerkeleyItem.h"
#include "core/MojObjectBuilder.h"

MojDbBerkeleyItem::MojDbBerkeleyItem()
: m_free(MojFree)
{
	MojZero(&m_dbt, sizeof(DBT));
	m_dbt.flags = DB_DBT_REALLOC;
}

MojErr MojDbBerkeleyItem::kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine)
{
	MojErr err = m_header.read(kindEngine);
	MojErrCheck(err);

	kindIdOut = m_header.kindId();

	return MojErrNone;
}

MojErr MojDbBerkeleyItem::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected) const
{
	MojErr err = MojErrNone;
	MojTokenSet tokenSet;
	if (headerExpected) {
		err = m_header.visit(visitor, kindEngine);
		MojErrCheck(err);
		const MojString& kindId = m_header.kindId();
		err = kindEngine.tokenSet(kindId, tokenSet);
		MojErrCheck(err);
		m_header.reader().tokenSet(&tokenSet);
		err = m_header.reader().read(visitor);
		MojErrCheck(err);
	} else {
		MojObjectReader reader(data(), size());
		err = reader.read(visitor);
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojDbBerkeleyItem::id(const MojObject& id)
{
	m_header.reset();
	m_header.id(id);
	m_header.reader().data(data(), size());
}

void MojDbBerkeleyItem::clear()
{
	freeData();
	m_free = MojFree;
	m_dbt.data = NULL;
	m_dbt.size = 0;
}

bool MojDbBerkeleyItem::hasPrefix(const MojDbKey& prefix) const
{
	return (size() >= prefix.size() &&
	MojMemCmp(data(), prefix.data(), prefix.size()) == 0);
}

MojErr MojDbBerkeleyItem::toArray(MojObject& arrayOut) const
{
	MojObjectBuilder builder;
	MojErr err = builder.beginArray();
	MojErrCheck(err);
	err = MojObjectReader::read(builder, data(), size());
	MojErrCheck(err);
	err = builder.endArray();
	MojErrCheck(err);
	arrayOut = builder.object();

	return MojErrNone;
}

MojErr MojDbBerkeleyItem::toObject(MojObject& objOut) const
{
	MojObjectBuilder builder;
	MojErr err = MojObjectReader::read(builder, data(), size());
	MojErrCheck(err);
	objOut = builder.object();

	return MojErrNone;
}

void MojDbBerkeleyItem::fromBytesNoCopy(const MojByte* bytes, MojSize size)
{
	MojAssert(bytes || size == 0);
	MojAssert(size <= MojUInt32Max);

	setData(const_cast<MojByte*> (bytes), size, NULL);
}

MojErr MojDbBerkeleyItem::fromBuffer(MojBuffer& buf)
{
	clear();
	if (!buf.empty()) {
		MojErr err = buf.release(m_chunk);
		MojErrCheck(err);
		MojAssert(m_chunk.get());
		setData(m_chunk->data(), m_chunk->dataSize(), NULL);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyItem::fromBytes(const MojByte* bytes, MojSize size)
{
	MojAssert (bytes || size == 0);

	if (size == 0) {
		clear();
	} else {
		MojByte* newBytes = (MojByte*)MojMalloc(size);
		MojAllocCheck(newBytes);
		MojMemCpy(newBytes, bytes, size);
		setData(newBytes, size, MojFree);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyItem::fromObject(const MojObject& obj)
{
	MojObjectWriter writer;
	MojErr err = obj.visit(writer);
	MojErrCheck(err);
	err = fromBuffer(writer.buf());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyItem::fromObjectVector(const MojVector<MojObject>& vec)
{
	MojAssert(!vec.empty());

	MojObjectWriter writer;
	for (MojVector<MojObject>::ConstIterator i = vec.begin();
		 i != vec.end();
	++i) {
		MojErr err = i->visit(writer);
		MojErrCheck(err);
	}
	fromBuffer(writer.buf());

	return MojErrNone;
}

void MojDbBerkeleyItem::freeData()
{
	if (m_free)
		m_free(m_dbt.data);
}

void MojDbBerkeleyItem::setData(MojByte* bytes, MojSize size, void (*free)(void*))
{
	MojAssert(bytes);
	freeData();
	m_free = free;
	m_dbt.size = (MojUInt32) size;
	m_dbt.data = bytes;

	m_header.reader().data(bytes, size);
}
