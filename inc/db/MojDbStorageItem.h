#pragma once

#include "db/MojDbDefs.h"
#include "core/MojRefCount.h"
#include "core/MojObject.h"

class MojDbStorageItem : public MojRefCounted
{
public:
	virtual ~MojDbStorageItem() {}
	virtual MojErr close() = 0;
	virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine) = 0;
	virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const = 0;
	virtual const MojObject& id() const = 0;
	virtual MojSize size() const = 0;

	MojErr toObject(MojObject& objOut, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
	MojErr toJson(MojString& strOut, MojDbKindEngine& kindEngine) const;

protected:
	MojObject m_id;
};
