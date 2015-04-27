#include "db/MojDbStorageItem.h"
#include "core/MojObjectBuilder.h"
#include "core/MojJson.h"

MojErr MojDbStorageItem::toObject(MojObject& objOut, MojDbKindEngine& kindEngine, bool headerExpected) const
{
	MojObjectBuilder builder;
	MojErr err = visit(builder, kindEngine, headerExpected);
	MojErrCheck(err);
	objOut = builder.object();
	return MojErrNone;
}

MojErr MojDbStorageItem::toJson(MojString& strOut, MojDbKindEngine& kindEngine) const
{
	MojJsonWriter writer;
	MojErr err = visit(writer, kindEngine);
	MojErrCheck(err);
	strOut = writer.json();
	return MojErrNone;
}
