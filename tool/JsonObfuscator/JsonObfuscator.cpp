/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2015 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

#include "JsonObfuscator.h"

#include <iostream>
#include <list>
#include <boost/regex.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp> // include Boost, a C++ library

const JsonObfuscate::char_array JsonObfuscate::ch_set { charset() };

JsonObfuscate::char_array JsonObfuscate::charset()
{
	//Change this to suit
	return char_array( {
		'0', '1', '2', '3', '4',
		'5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F',
		'G', 'H', 'I', 'J', 'K',
		'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U',
		'V', 'W', 'X', 'Y', 'Z',
		'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'n', 'o', 'p',
		'q', 'r', 's', 't', 'u',
		'v', 'w', 'x', 'y', 'z'
	});
};

JsonObfuscate::JsonObfuscate()
: rng(std::random_device {}()),
  dist(0, static_cast<int>(ch_set.size() - 1))
{
    randchar = [this]() {
        return ch_set[ dist(rng) ];
    };
}

std::string JsonObfuscate::random_string(std::size_t length)
{
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::string JsonObfuscate::getValue(const std::string &value)
{
    if (value.empty())
        return "";
    else if (value.size() == 1)
        return value;

    auto i = Words.find(value);
    if (i == Words.end()) {
        std::string randstring = random_string(7);
        Words[value] = randstring;
        return randstring;
    } else {
        return i->second;
    }
}

void JsonObfuscate::run(const std::string &fnin, const std::string &fnout)
{

    std::ifstream infile(fnin);
    std::ofstream outfile(fnout, std::ios::trunc | std::ios::binary);

    boost::regex expression("\"([_a-zA-Z0-9\\-\\$\\/\\.]+)\"");
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    std::string::const_iterator start, end;

    std::string line;
    std::string resultline;

    while (std::getline(infile, line)) {
        start = line.begin();
        end = line.end();

        resultline = line;

        while (boost::regex_search(start, end, what, expression, flags)) {
            for (unsigned int i = 1; i < what.size(); ++i) {
                std::string source = what[i];
                std::string replace_to = getValue(what[i]);

                boost::replace_all(resultline, "\"" + source + "\"", "\"" + replace_to + "\"");
            }

            start = what[0].second;
            flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;
        }

        outfile << resultline << std::endl;
    }
}
