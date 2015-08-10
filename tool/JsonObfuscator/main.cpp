#include "JsonObfuscator.h"
#include <iostream>

void printHelp()
{
	std::cerr << "Usage: mojoobfuscate [infile] [outfile]" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Where: " << std::endl;
	std::cerr << "\tinfile : Path to json file to be obfuscated" << std::endl;
	std::cerr << "\toutfile : Path to obfuscated result file" << std::endl;
}


int main(int argc, const char** argv)
{
	if (argc != 3) {
		std::cerr << "Not enough params" << std::endl;
		printHelp();
		return -1;
	}

	JsonObfuscate obfuscator;
	try {
		obfuscator.run(argv[1], argv[2]);
	} catch (const std::exception& error) {
		std::cerr << "Error: " << error.what() << std::endl;

		return -1;
	}

	return -2;
}
