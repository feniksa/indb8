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

#include "db/MojDb.h"
#include "core/MojUtil.h"
#include "core/MojJson.h"
#include "core/MojObjectBuilder.h"

#include <vector>
#include <random>
#include <functional> //for std::function
#include <algorithm>  //for std::generate_n

#include <iostream>
#include <list>
#include <boost/regex.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp> // include Boost, a C++ library

static const MojChar* const MojInputFileName = _T("/tmp/defaultSettings.json");
static const MojChar* const MojOutputFileName = _T("/tmp/defaultSettingsObfuscated.json");

using namespace std;

struct JsonObfuscate
{
	using rand_char_t = std::function<char(void)> ;
	using char_array = std::vector<char>;

	const char_array ch_set;

	std::default_random_engine rng;
	std::uniform_int_distribution<> dist;
	rand_char_t randchar;

	std::map<std::string, std::string> Words;

	char_array charset()
	{
		//Change this to suit
		return char_array(
			{'0','1','2','3','4',
				'5','6','7','8','9',
				'A','B','C','D','E','F',
				'G','H','I','J','K',
				'L','M','N','O','P',
				'Q','R','S','T','U',
				'V','W','X','Y','Z',
				'a','b','c','d','e','f',
				'g','h','i','j','k',
				'l','m','n','o','p',
				'q','r','s','t','u',
				'v','w','x','y','z'
			});
	};

	JsonObfuscate()
	: ch_set(charset()),
	  rng(std::random_device{}()),
	  dist(0, static_cast<int> (ch_set.size()-1))
	{
		randchar = [this](){ return ch_set[ dist(rng) ]; };
	}

	std::string random_string( size_t length)
	{
		std::string str(length, 0);
		std::generate_n( str.begin(), length, randchar );
		return str;
	}

	std::string getValue(const std::string& value)
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

	int run(int argc, const char** argv);
};

void printHelp()
{
	std::cerr << "Usage: mojoobfuscate [infile] [outfile]" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Where: " << std::endl;
	std::cerr << "\tinfile : Path to json file to be obfuscated" << std::endl;
	std::cerr << "\toutfile : Path to obfuscated result file" << std::endl;
}

int JsonObfuscate::run(int argc, const char** argv)
{
	if (argc != 3) {
		std::cerr << "Not enough params" << std::endl;
		printHelp();
		return -1;
	}

	std::ifstream infile(argv[1]);
	std::ofstream outfile(argv[2], std::ios::trunc | std::ios::binary);

	boost::regex expression("\"([_a-zA-Z0-9\\-\\$\\/\\.]+)\"");
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;

	std::string::const_iterator start, end;

	std::string line;
	std::string resultline;

	while (std::getline(infile, line))
	{
		start = line.begin();
		end = line.end();

		resultline = line;

		while (boost::regex_search(start, end, what, expression, flags)) {
			for (unsigned int i = 1; i < what.size(); ++i) {
				//std::cout << "Found: " << " " << what[i] << std::endl;

				std::string source = what[i];
				std::string replace_to = getValue(what[i]);

				boost::replace_all(resultline,"\"" + source + "\"", "\"" + replace_to + "\"");
			}

			start = what[0].second;
			flags |= boost::match_prev_avail;
			flags |= boost::match_not_bob;
		}

		outfile << resultline << std::endl;
	}

	return 0;
}

int main(int argc, const char** argv)
{
	JsonObfuscate obfuscator;
	try {
		return obfuscator.run(argc, argv);
	} catch (const std::exception& error) {
		std::cerr << "Error: " << error.what() << std::endl;
		return -1;
	}

	return -2;
}
