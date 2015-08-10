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

#include "core/MojJsonParser.h"

static const MojChar* const MojJsonNullString = _T("null");
static const MojChar* const MojJsonTrueString = _T("true");
static const MojChar* const MojJsonFalseString = _T("false");

MojJsonParser::MojJsonParser()
: m_line(0),
  m_col(0),
  m_strPos(0),
  m_depth(0),
  m_ucsChar(0),
  m_matchStr(NULL),
  m_isDecimal(false)
{
    MojZero(m_stack, sizeof(m_stack));
}

MojJsonParser::~MojJsonParser()
{
}

void MojJsonParser::begin()
{
    m_line = 1;
    m_col = 1;
    resetRec();
}

MojErr MojJsonParser::end(MojObjectVisitor& visitor)
{
    MojChar c = _T('\0');
    const MojChar* parseEnd = NULL;
    MojErr err = parseChunk(visitor, &c, 1, parseEnd);
    MojErrCheck(err);

    return MojErrNone;
}

bool MojJsonParser::finished() const
{
    return (state() == State::Finish && m_depth == 0);
}

MojErr MojJsonParser::parse(MojObjectVisitor& visitor, const MojChar* chars, MojSize len)
{
    MojAssert(chars || len == 0);

    MojJsonParser parser;
    MojErr err;
    parser.begin();

    const MojChar* parseEnd = NULL;
    err = parser.parseChunk(visitor, chars, len, parseEnd);
    MojErrCheck(err);

    err = parser.end(visitor);
    MojErrCheck(err);

    if (!parser.finished())
        MojErrThrow(MojErrJsonParseEof);

    return MojErrNone;
}

MojErr MojJsonParser::parseChunk(MojObjectVisitor& visitor, const MojChar* chars, MojSize len, const MojChar*& parseEnd)
{
    MojErr err = MojErrNone;
    const MojChar* end = (len == MojSizeMax) ? NULL : chars + len;

    MojChar c;
    while (chars != end) {
        c = *chars;
        if (c == '\n') {
            ++m_line;
            m_col = 1;
        }

Redo:   switch (state()) {
        case State::EatWhitespace:
            if (MojIsSpace(c)) {
                /* okay */
            } else if (c == '/') {
                state() = State::CommentStart;
            } else {
                state() = savedState();
                goto Redo;
            }
            break;

		case State::Start:
            switch (c) {
            case '{':
                state() = State::EatWhitespace;
                savedState() = State::ObjFieldStart;
                err = visitor.beginObject();
                MojErrCheck(err);
                break;
            case '[':
                state() = State::EatWhitespace;
                savedState() = State::Array;
                err = visitor.beginArray();
                MojErrCheck(err);
                break;
            case 'n':
                state() = State::Null;
                m_strPos = 0;
                goto Redo;
            case '"':
                state() = State::String;
                sclear();
                break;
            case 't':
            case 'f':
                state() = State::Bool;
                m_strPos = 0;
                m_matchStr = (c == 't') ? MojJsonTrueString : MojJsonFalseString;
                goto Redo;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                state() = State::Number;
                sclear();
                m_isDecimal = false;
                goto Redo;
            case '\0':
                break;
            default:
                MojErrThrowMsg(MojErrJsonParseUnexpected, _T("json: unexpected char at %d:%d"), m_line, m_col);
            }
            break;

			case State::Finish:
            if (m_depth == 0)
                goto Done;
            m_depth--;
            goto Redo;

			case State::Null:
            if (c != MojJsonNullString[m_strPos])
                MojErrThrowMsg(MojErrJsonParseNull, _T("json: error parsing null at %d:%d"), m_line, m_col);
            if (MojJsonNullString[++m_strPos] == _T('\0')) {
                err = visitor.nullValue();
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            }
            break;

		case State::CommentStart:
            if (c == '*') {
                state() = State::Comment;
            } else if (c == '/') {
                state() = State::CommentEol;
            } else {
                MojErrThrow(MojErrJsonParseComment);
            }
            break;

		case State::Comment:
            if (c == '*')
                state() = State::CommentEnd;
            break;

		case State::CommentEol:
            if (c == '\n') {
                state() = State::EatWhitespace;
            }
            break;

		case State::CommentEnd:
            if (c == '/') {
                state() = State::EatWhitespace;
            } else {
                state() = State::Comment;
                goto Redo;
            }
            break;

		case State::String:
            if (c == '"') {
                err = visitor.stringValue(m_str, m_str.length());
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            } else if (c == '\\') {
                savedState() = State::String;
                state() = State::StringEscape;
            } else {
                err = sappend(c);
                MojErrCheck(err);
            }
            break;

		case State::StringEscape:
            if (c == 'u'){
                m_ucsChar = 0;
                m_strPos = 0;
                state() = State::EscapeUnicode;
            } else {
                MojChar escapeChar;
                switch (c) {
                case '"':
                case '\\':
                case '/':
                    escapeChar = c;
                    break;
                case 'b':
                    escapeChar = _T('\b');
                    break;
                case 'n':
                    escapeChar = _T('\n');
                    break;
                case 'r':
                    escapeChar = _T('\r');
                    break;
                case 't':
                    escapeChar = _T('\t');
                    break;
                case 'f':
                    escapeChar = _T('\f');
                    break;
                default:
                    MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing string at %d:%d"), m_line, m_col);
                }
				err = sappend(escapeChar);
                MojErrCheck(err);
                state() = savedState();
            }
            break;

		case State::EscapeUnicode:
            if (MojIsHexDigit(c)) {
                m_ucsChar += ((MojUInt32) hexDigit(c) << ((3 - m_strPos++) * 4));
                if (m_strPos == 4) {
                    MojChar utfOut[3];
                    MojSize utfLen = 0;
                    if (m_ucsChar == 0) {
                        MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing escape sequence - null character not allowed at %d:%d"), m_line, m_col);
                    }
                    if (m_ucsChar < 0x80) {
                        utfOut[0] = (MojChar) m_ucsChar;
                        utfLen = 1;
                    } else if (m_ucsChar < 0x800) {
                        utfOut[0] = (MojChar) (0xc0 | (m_ucsChar >> 6));
                        utfOut[1] = (MojChar) (0x80 | (m_ucsChar & 0x3f));
                        utfLen = 2;
                    } else {
                        utfOut[0] = (MojChar) (0xe0 | (m_ucsChar >> 12));
                        utfOut[1] = (MojChar) (0x80 | ((m_ucsChar >> 6) & 0x3f));
                        utfOut[2] = (MojChar) (0x80 | (m_ucsChar & 0x3f));
                        utfLen = 3;
                    }
                    err = sappend(utfOut, utfLen);
                    MojErrCheck(err);
                    state() = savedState();
                }
            } else {
                MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing escape sequence at %d:%d"), m_line, m_col);
            }
            break;

		case State::Bool:
            if (c != m_matchStr[m_strPos])
                MojErrThrowMsg(MojErrJsonParseBool, _T("json: error parsing bool at %d:%d"), m_line, m_col);
            if (m_matchStr[++m_strPos] == _T('\0')) {
                err = visitor.boolValue(m_matchStr == MojJsonTrueString);
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            }
            break;

		case State::Number:
            switch (c) {
            case '.':
            case 'e':
            case 'E':
                m_isDecimal = true;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-': {
                err = sappend(c);
                MojErrCheck(err);
                break;
            }
            default: {
                const MojChar* numberEnd;
                if (m_isDecimal) {
                    MojDecimal d;
                    err = d.assign(m_str);
                    MojErrCheck(err);
                    err = visitor.decimalValue(d);
                    MojErrCheck(err);
                } else {
                    MojInt64 i = MojStrToInt64(m_str, &numberEnd, 0);
                    err = visitor.intValue(i);
                    MojErrCheck(err);
                    if (numberEnd != m_str.end())
                        MojErrThrowMsg(MojErrJsonParseInt, _T("json: error parsing int at %d:%d"), m_line, m_col);
                }
                savedState() = State::Finish;
                state() = State::EatWhitespace;
                goto Redo;
            }
            }
            break;

			case State::Array:
            if (c == ']') {
                err = visitor.endArray();
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            } else {
                state() = State::ArraySep;
                err = push();
                MojErrCheck(err);
                goto Redo;
            }
            break;

		case State::ArraySep:
            if (c == ']') {
                err = visitor.endArray();
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            } else if (c == ',') {
                savedState() = State::Array;
                state() = State::EatWhitespace;
            } else {
                MojErrThrowMsg(MojErrJsonParseArray, _T("json: error parsing array at %d:%d"), m_line, m_col);
            }
            break;

		case State::ObjFieldStart:
            if (c == '}') {
                err = visitor.endObject();
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            } else if (c == '"') {
				sclear();
                state() = State::ObjField;
            } else {
                MojErrThrowMsg(MojErrJsonParsePropName, _T("json: error parsing prop name at %d:%d"), m_line, m_col);
            }
            break;

		case State::ObjField:
            if (c == '"') {
                err = visitor.propName(m_str, m_str.length());
                MojErrCheck(err);
                savedState() = State::ObjFieldEnd;
                state() = State::EatWhitespace;
            } else if (c == '\\') {
                savedState() = State::ObjField;
                state() = State::StringEscape;
            } else {
                err = sappend(c);
                MojErrCheck(err);
            }
            break;

		case State::ObjFieldEnd:
            if (c == ':') {
                savedState() = State::ObjValue;
                state() = State::EatWhitespace;
            } else {
                MojErrThrowMsg(MojErrJsonParsePropName, _T("json: error parsing prop name at %d:%d"), m_line, m_col);
            }
            break;

        case State::ObjValue:
            savedState() = State::ObjSep;
            state() = State::EatWhitespace;
            err = push();
            MojErrCheck(err);
            goto Redo;

        case State::ObjSep:
            if (c == '}') {
                err = visitor.endObject();
                MojErrCheck(err);
                savedState() = State::Finish;
                state() = State::EatWhitespace;
            } else if (c == ',') {
                savedState() = State::ObjFieldStart;
                state() = State::EatWhitespace;
            } else {
                MojErrThrowMsg(MojErrJsonParseValueSep, _T("json: error parsing value separator at %d:%d"), m_line, m_col);
            }
            break;

        default:
            MojAssertNotReached();
        }
        ++chars;
        ++m_col;
    }

Done:
    parseEnd = chars;
    return MojErrNone;
}

MojErr MojJsonParser::push()
{
    if (m_depth >= MaxDepth - 1)
        MojErrThrowMsg(MojErrJsonParseMaxDepth, _T("json: max depth exceeded at %d:%d"), m_line, m_col);
    m_depth++;
    resetRec();
    return MojErrNone;
}

void MojJsonParser::resetRec()
{
    state() = State::EatWhitespace;
    savedState() = State::Start;
}
