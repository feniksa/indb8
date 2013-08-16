/* @@@LICENSE
*
*      Copyright (c) 2013 LG Electronics, Inc.
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


#ifndef MOJDBSHARDENGINE_H_
#define MOJDBSHARDENGINE_H_

#include "core/MojCoreDefs.h"
#include "core/MojErr.h"
#include "core/MojString.h"

class MojDb;

class MojDbShardEngine : private MojNoCopy
{
public:
    struct ShardInfo
    {
        bool active;
        bool transient;
        MojUInt32 id;
        MojString path;
        MojString media;
        MojString id_base64;

        ShardInfo()
        {
            id = 0;
            active = false;
            transient = false;
        }
    };

    MojDbShardEngine(void);
    ~MojDbShardEngine(void);

    //init
    MojErr init (MojDb* ip_db);

    //put a new shard description to db
    MojErr put (ShardInfo& i_info);
    MojErr put (MojUInt32 i_id, bool i_active, bool i_transient, MojString& i_path, MojString& i_media, MojString& i_id_base64);

    //get shard description by id
    MojErr get (MojUInt32 i_id, ShardInfo& o_info);

    //get correspond shard id using path to device
    MojErr getIdForPath (MojString& i_path, MojUInt32& o_id);

    //set shard activity
    MojErr setActivity (MojUInt32 i_id, bool i_isActive);

    //compute a new shard id
    MojErr computeShardId (MojString& i_media, MojUInt32& o_id);

    //return: MojErrExists         --> exist
    //        MojErrNotFound       --> not found
    //        MojErrNotInitialized --> db was not initialized
    MojErr isIdExist (MojUInt32 i_id);

    //return: MojErrExists         --> exist
    //        MojErrNotFound       --> not found
    //        MojErrNotInitialized --> db was not initialized
    MojErr isIdExist (MojString i_id_base64);

private:
    MojDb* mp_db;
    static MojLogger s_log;

    bool _computeId (MojString& i_media, MojUInt32& o_id);
};

inline MojErr MojDbShardEngine::put (ShardInfo& i_info)
{
    return put(i_info.id, i_info.active, i_info.transient, i_info.path, i_info.media, i_info.id_base64);
}

#endif /* MOJDBSHARDENGINE_H_ */
