/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */


#include "db/MojDbClient.h"

MojErr MojDbClient::putPermission(Signal::SlotRef handler, const MojObject& obj)
{
	MojErr err = putPermissions(handler, &obj, &obj+1);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClient::put(Signal::SlotRef handler, const MojObject& obj, MojUInt32 flags)
{
	MojErr err = put(handler, &obj, &obj+1, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClient::get(Signal::SlotRef handler, const MojObject& id)
{
	MojErr err = get(handler, &id, &id+1);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClient::del(Signal::SlotRef handler, const MojObject& id, MojUInt32 flags)
{
	MojErr err = del(handler, &id, &id + 1, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClient::merge(Signal::SlotRef handler, const MojObject& obj, MojUInt32 flags)
{
	MojErr err = merge(handler, &obj, &obj + 1, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBatch::put(const MojObject& obj, MojUInt32 flags)
{
	MojErr err = put(&obj, &obj + 1, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBatch::get(const MojObject& id)
{
	MojErr err = get(&id, &id + 1);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBatch::del(const MojObject& id, MojUInt32 flags)
{
	MojErr err = del(&id, &id + 1, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBatch::merge(const MojObject& obj, MojUInt32 flags)
{
	MojErr err = merge(&obj, &obj + 1, flags);
	MojErrCheck(err);

	return MojErrNone;
}
