#pragma once

#include <vector>
#include <random>
#include <functional> //for std::function
#include <algorithm>  //for std::generate_n
#include <map>
#include <string>

class JsonObfuscate
{
	using rand_char_t = std::function<char(void)> ;
	using char_array = std::vector<char>;

public:
	JsonObfuscate();

	void run(const std::string& infile, const std::string& outfile);

private:
	static char_array charset();
	std::string random_string( size_t length);
	std::string getValue(const std::string& value);

	static const char_array ch_set;

	std::default_random_engine rng;
	std::uniform_int_distribution<> dist;
	rand_char_t randchar;

	std::map<std::string, std::string> Words;
};
