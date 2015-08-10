#pragma once

#include <string>
#include "core/MojCoreDefs.h"

class MojStdString : public std::string
{
public:
	using std::string::string;

	inline MojErr reserve(MojSize size) { std::string::reserve(size); return MojErrNone; }
	inline MojErr append(MojChar c) { std::string::operator+=(c); return MojErrNone; }
	inline MojErr append(const MojChar* str, MojSize size) { std::string::append(str, size); return MojErrNone; }
	inline MojSize length() const { return std::string::size(); }
	inline operator const MojChar*() const { return data(); }

	inline const char* end() { return &(*(std::string::end())); /* <-- not bug, it should point to address of \0 */ }
};
