/* @@@LICENSE
 *
 *      Copyright (c) 2009-2015 LG Electronics, Inc.
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

#pragma once

#include "core/MojCoreDefs.h"
#include "core/MojObject.h"

class MojJsonWriter : public MojObjectVisitor
{
public:
    using MojObjectVisitor::propName;

    MojJsonWriter();

    virtual MojErr reset();
    virtual MojErr beginObject();
    virtual MojErr endObject();
    virtual MojErr beginArray();
    virtual MojErr endArray();
    virtual MojErr propName(const MojChar* name, MojSize len);
    virtual MojErr nullValue();
    virtual MojErr boolValue(bool val);
    virtual MojErr intValue(MojInt64 val);
    virtual MojErr decimalValue(const MojDecimal& val);
    virtual MojErr stringValue(const MojChar* val, MojSize len);

    const MojString& json() const { return m_str; }

private:
    static const MojSize InitialSize = 1024;

    MojErr writeString(const MojChar* val, MojSize len);
    MojErr writeComma();

    bool m_writeComma;
    MojString m_str;
};
