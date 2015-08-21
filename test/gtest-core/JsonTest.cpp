/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013-2015 LG Electronics, Inc.
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


#include "gtest/gtest.h"

#include "db/MojDb.h"
#include "core/MojUtil.h"
#include "core/MojJson.h"
#include "core/MojObjectBuilder.h"

#include "Runner.h"

static const MojChar* const MojDefaultSettingsFileName = DEFAULT_SETTINGS_PATH;

#include <iostream>
#include <list>
using namespace std;

struct JsonTest : public ::testing::Test
{
	void SetUp()
	{
		MojStatT mojStatFile;
		MojErr err = MojStat(MojDefaultSettingsFileName, &mojStatFile);
		MojAssertNoErr(err);
	}

	void TearDown()
	{
	}
};

/**
 * Load MojDefaultSettingsFileName and just parse it.
 */
TEST_F(JsonTest, paser_json_withoutobj)
{
	MojErr err;
	MojFile file;

	MojJsonParser parser;
	parser.begin();
	MojSize bytesRead = 0;
	MojObjectEater visitor;

	MojAssertNoErr(file.open(MojDefaultSettingsFileName, 0));

	unsigned int parsedChunks = 0;
	do {
		MojChar buf[MojFile::MojFileBufSize];
		err = file.read(buf, sizeof(buf), bytesRead);
		MojAssertNoErr(err);

		const MojChar* parseEnd = buf;
		while (parseEnd < (buf + bytesRead)) {
			err = parser.parseChunk(visitor, parseEnd, bytesRead - (parseEnd - buf), parseEnd);

			++parsedChunks;

			MojAssertNoErr(err);
			if (parser.finished()) {
				parser.begin();
				visitor.reset();
			}
		}
	} while (bytesRead > 0);

	cout << "Parsed chunks: " << parsedChunks << endl;
}

/**
 * Load MojDefaultSettingsFileName, parse json file and build MojObject
 */
TEST_F(JsonTest, paser_json_withobj)
{
	MojErr err;
	MojFile file;

	MojJsonParser parser;
	parser.begin();
	MojSize bytesRead = 0;
	MojObjectBuilder visitor;

	MojAssertNoErr(file.open(MojDefaultSettingsFileName, 0));

	unsigned int parsedChunks = 0;
	do {
		MojChar buf[MojFile::MojFileBufSize];
		err = file.read(buf, sizeof(buf), bytesRead);
		MojAssertNoErr(err);

		const MojChar* parseEnd = buf;
		while (parseEnd < (buf + bytesRead)) {
			err = parser.parseChunk(visitor, parseEnd, bytesRead - (parseEnd - buf), parseEnd);

			++parsedChunks;

			MojAssertNoErr(err);
			if (parser.finished()) {
				parser.begin();
				visitor.reset();
			}
		}
	} while (bytesRead > 0);

	cout << "Parsed chunks: " << parsedChunks << endl;
}
