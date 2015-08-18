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

/*
 * Based on the json-c library:
 * Copyright (c) 2004, 2005 Metaparadigm Pte Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * Done OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "core/MojJsonWriter.h"

static const MojChar* const MojJsonNullString = _T("null");
static const MojChar* const MojJsonTrueString = _T("true");
static const MojChar* const MojJsonFalseString = _T("false");

MojJsonWriter::MojJsonWriter()
: m_writeComma(false)
{
}

MojErr MojJsonWriter::reset()
{
    m_str.clear();
    m_writeComma = false;
    return MojErrNone;
}

MojErr MojJsonWriter::beginObject()
{
    MojErr err = m_str.reserve(InitialSize);
    MojErrCheck(err);
    err = writeComma();
    MojErrCheck(err);
    err = m_str.append(_T('{'));
    MojErrCheck(err);
    m_writeComma = false;
    return MojErrNone;
}

MojErr MojJsonWriter::endObject()
{
    MojErr err = m_str.append(_T('}'));
    MojErrCheck(err);
    m_writeComma = true;
    return MojErrNone;
}

MojErr MojJsonWriter::beginArray()
{
    MojErr err = writeComma();
    MojErrCheck(err);
    err = m_str.append(_T('['));
    MojErrCheck(err);
    m_writeComma = false;
    return MojErrNone;
}

MojErr MojJsonWriter::endArray()
{
    MojErr err = m_str.append(_T(']'));
    MojErrCheck(err);
    m_writeComma = true;
    return MojErrNone;
}

MojErr MojJsonWriter::propName(const MojChar* name, MojSize len)
{
    MojAssert(name);

    MojErr err = writeComma();
    MojErrCheck(err);
    err = writeString(name, len);
    MojErrCheck(err);
    err = m_str.append(_T(':'));
    MojErrCheck(err);
    m_writeComma = false;
    return MojErrNone;
}

MojErr MojJsonWriter::nullValue()
{
    MojErr err = writeComma();
    MojErrCheck(err);
    err = m_str.append(MojJsonNullString);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojJsonWriter::boolValue(bool val)
{
    MojErr err = writeComma();
    MojErrCheck(err);
    err = m_str.append(val ? MojJsonTrueString : MojJsonFalseString);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojJsonWriter::intValue(MojInt64 val)
{
    MojErr err = writeComma();
    MojErrCheck(err);
    err = m_str.appendFormat(_T("%lld"), val);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojJsonWriter::decimalValue(const MojDecimal& val)
{
    MojErr err = writeComma();
    MojErrCheck(err);
    MojChar buf[MojDecimal::MaxStringSize];
    err = val.stringValue(buf, MojDecimal::MaxStringSize);
    MojErrCheck(err);
    err = m_str.append(buf);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojJsonWriter::stringValue(const MojChar* val, MojSize len)
{
    MojErr err = writeComma();
    MojErrCheck(err);
    err = writeString(val, len);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojJsonWriter::writeString(const MojChar* val, MojSize len)
{
    MojAssert(val || len == 0);
    static const char escape[] = {
        /*       0,  1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
        /* 0 */  1,  1,  1,  1,  1,  1,  1,  1,'b','t','n',  1,'f','r',  1,  1,
        /* 1 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        /* 2 */  0,  0,'"',  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 3 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 4 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 5 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,'\\', 0,  0,  0
    };

    MojErr err = m_str.append(_T('"'));
    MojErrCheck(err);

    const MojChar* cur = val;
    const MojChar* end = val + len;
    while (cur < end) {
        unsigned int c = (unsigned int) *cur;
        if (c < (int) sizeof(escape) && escape[c]) {
            err = m_str.append(val, cur - val);
            MojErrCheck(err);
            val = cur + 1;
            if (escape[c] == 1) {
                err = m_str.appendFormat(_T("\\u%04X"), c);
                MojErrCheck(err);
            } else {
                err = m_str.append(_T('\\'));
                MojErrCheck(err);
                err = m_str.append(escape[c]);
                MojErrCheck(err);
            }
        }
        cur++;
    }
    err = m_str.append(val, cur - val);
    MojErrCheck(err);
    err = m_str.append(_T('"'));
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojJsonWriter::writeComma()
{
    if (m_writeComma) {
        MojErr err = m_str.append(_T(','));
        MojErrCheck(err);
    } else {
        m_writeComma = true;
    }
    return MojErrNone;
}
