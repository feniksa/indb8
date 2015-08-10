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

class MojJsonParser
{
	enum class State {
		EatWhitespace,
		Start,
		Finish,
		Null,
		CommentStart,
		Comment,
		CommentEol,
		CommentEnd,
		String,
		StringEscape,
		EscapeUnicode,
		Bool,
		Number,
		Array,
		ArraySep,
		ObjFieldStart,
		ObjField,
		ObjFieldEnd,
		ObjValue,
		ObjSep
	};

	struct StackRec
	{
		State m_state;
		State m_savedState;
	};

public:
    MojJsonParser();
    ~MojJsonParser();

    void begin();
    MojErr end(MojObjectVisitor& visitor);

	bool finished() const;

    static MojErr parse(MojObjectVisitor& visitor, const MojChar* str) { return parse(visitor, str, MojSizeMax); }
    static MojErr parse(MojObjectVisitor& visitor, const MojChar* chars, MojSize len);
    MojErr parseChunk(MojObjectVisitor& visitor, const MojChar* chars, MojSize len, const MojChar*& parseEnd);

    inline MojUInt32 line() const { return m_line; }
    inline MojUInt32 column() const { return m_col; }

private:
    static const MojSize MaxDepth = 32;

    inline State& state() { return m_stack[m_depth].m_state; }
    inline const State& state() const { return m_stack[m_depth].m_state; }

    inline State& savedState() { return m_stack[m_depth].m_savedState; }
    inline const State& savedState() const { return m_stack[m_depth].m_savedState; }

    inline int hexDigit(MojChar c) { return (c <= _T('9')) ? c - _T('0') : (c & 7) + 9; }

    MojErr push();
    void resetRec();

    MojUInt32 m_line;
    MojUInt32 m_col;
    MojSize m_strPos;
    MojSize m_depth;
    MojUInt32 m_ucsChar;
    const MojChar* m_matchStr;
    bool m_isDecimal;
    MojString m_str;
    StackRec m_stack[MaxDepth];
};

