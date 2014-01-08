/* @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
#ifndef MOJDBMEDIALINKMANAGER_H
#define MOJDBMEDIALINKMANAGER_H

#include "core/MojString.h"
#include "db/MojDbShardInfo.h"

class MojDbMediaLinkManager
{
public:
    MojDbMediaLinkManager();
    MojErr setLinkDirectory(const MojString& dir);

    MojErr processShardInfo(MojDbShardInfo& shardInfo);
    MojErr createLink(MojDbShardInfo& shardInfo);
    MojErr removeLink(const MojDbShardInfo& shardInfo);
private:
    MojErr getLinkPath(MojUInt32 shardId, MojString& linkPath);

    MojString m_dir;

    static MojLogger s_log;
};

#endif
