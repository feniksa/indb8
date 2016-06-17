/* @@@LICENSE
*
*      Copyright (c) 2009-2014 LG Electronics, Inc.
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

/**
****************************************************************************************************
* Filename              : MojUtilTest.cpp

* Description           : Source file for MojUtil test.
****************************************************************************************************
**/
#include "MojUtilTest.h"
#include <unistd.h>

static const MojChar* const TestFileName = _T("bartestfile");

MojUtilTest::MojUtilTest()
: MojTestCase(_T("MojUtil"))
{
}

/**
***************************************************************************************************
* @run                    This function tests the utility functions provided in DB8.These include
                          the following:
                          1.MojMin()/MojMax:
                            a)MojMin():This returns the minimum of two given numbers.
                              eg:MojMin(1,8) //returns one.
                            b)MojMax():This returns the maximum of two given numbers.
                              eg:MojMax(14,2) //returns 14
                           2.MojFlagGet/MojFlagSet
                             a)MojFlagGet():This function returns the flag set value or it returns
                               ((flags & mask) == mask);
                               eg:MojFlagGet(flags, 1))//returns true as flag is set to 1
                              b)MojFlagSet(): This sets the flag value or returns (flags |= mask;)
                                eg:MojFlagSet(flags, 1, true);//this sets the flag value as 1
                                   MojFlagSet(flags, 2, false);//this sets the flag value to 1 when the
                                   old flag value is 3.
                          3.MojMkDir/MojRmDirRecursive
                               a)MojMkDir():It creates a new Directory with the given mode.
                                 eg:err = MojMkDir(_T("foo/bar"), S_IRWXU);
                               b)MojRmDirRecursive());It removes the directory ,sub-directories and the
                                 contents inside the directory.
                                 err = MojRmDirRecursive(_T("foo"));
                          4.MojFileOpen/MojFileClose:
                               a)MojFileOpen():It opens the file in the mode and the flag specified
                                 eg:err = MojFileOpen(file, _T("foo/file2"), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
                               b)MojFileClose():It closes the opened file.
                                 eg:err = MojFileClose(file);
                          5.testBase64/testBase64Mime:
                               a)testBase64:It checks the encoding and decoding functionality of Base 64.
                                 Base64 is a group of similar binary-to-text encoding schemes that represent
                                 binary data in an ASCII string format by translating it into a radix-64 representation.
                             eg:MojByte input1[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e};
                                err = testBase64(input1, sizeof(input1), _T("4EiR+x_y"));
                               b)testBase64Mime: MIME's Base64 encoding is based on that of the RFC 1421 version.
                                 eg:MojByte input4[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e};
                                 err = testBase64Mime(input4, sizeof(input4), _T("FPucA9l+"));
                          6.MojBinarySearch/MojQuickSort:
                               a)MojBinarySearch:Binary search operation is done in this function.
                                 Invalid data also given and tested here.
                                 eg:int array3[] = {2,24,87};
                                 MojTestAssert(MojBinarySearch(0, array3, 3) == MojInvalidIndex);
                               b)MojQuickSort:Quick Sort functionality is done in this function
                          7.MojFileToString
                               The file contents will be stored in the given string
                               eg:err = MojFileToString(TestFileName, str);

* @param                : None
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::run()
{
	MojUInt32 flags = 0;
	MojFileT file;
	MojStatT stat;

	// min/max
	MojTestAssert(MojMin(1, 8) == 1);
	MojTestAssert(MojMin(5, 3) == 3);
	MojTestAssert(MojMax(1, 8) == 8);
	MojTestAssert(MojMax(14, 2) == 14);

	// flags
	MojFlagSet(flags, 1, true);
	MojTestAssert(flags == 1);
	MojTestAssert(MojFlagGet(flags, 1));
	MojTestAssert(!MojFlagGet(flags, 3));
	MojTestAssert(!MojFlagGet(flags, 8));
	MojFlagSet(flags, 2, true);
	MojTestAssert(flags == 3);
	MojFlagSet(flags, 2, false);
	MojTestAssert(flags == 1);

        //change cwd to /tmp
        //due to NFS behaviour - it reporting wrong folder attributes and make a problem
        MojTestAssert( chdir("/tmp") == 0 );
	// rmdirr
	MojErr err = MojMkDir(_T("foo"), S_IRWXU);
	MojTestErrCheck(err);
	err = MojMkDir(_T("foo/bar"), S_IRWXU);
	MojTestErrCheck(err);
	err = MojMkDir(_T("foo/hoo"), S_IRWXU);
	MojTestErrCheck(err);
	err = MojMkDir(_T("foo/hoo/yo"), S_IRWXU);
	MojTestErrCheck(err);
	err = MojFileOpen(file, _T("foo/file1"), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	MojTestErrCheck(err);
	err = MojFileClose(file);
	MojTestErrCheck(err);
	err = MojFileOpen(file, _T("foo/file2"), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	MojTestErrCheck(err);
	err = MojFileClose(file);
	MojTestErrCheck(err);
	err = MojFileOpen(file, _T("foo/bar/file2"), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	MojTestErrCheck(err);
	err = MojFileClose(file);
	MojTestErrCheck(err);
	err = MojCreateDirIfNotPresent("foo/zoom/zoom/boom");
	MojTestErrCheck(err);
	err = MojStat("foo/zoom/zoom/boom", &stat);
	MojTestErrCheck(err);
	err = MojRmDirRecursive(_T("foo"));
	MojTestErrCheck(err);
	err = MojStat(_T("foo"), &stat);
	MojTestErrExpected(err, MojErrNotFound);

	// base64
	err = testBase64(NULL, 0, _T(""));
	MojTestErrCheck(err);
	MojByte input1[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e};
	err = testBase64(input1, sizeof(input1), _T("4EiR+x_y"));
	MojTestErrCheck(err);
	MojByte input2[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9};
	err = testBase64(input2, sizeof(input2), _T("4EiR+xZ="));
	MojTestErrCheck(err);
	MojByte input3[] = {0x14, 0xfb, 0x9c, 0x03};
	err = testBase64(input3, sizeof(input3), _T("4EiR+k=="));
	MojTestErrCheck(err);
	err = testBase64Err(_T("FPucAw==="));
	MojTestErrCheck(err);
	err = testBase64Err(_T("FPucA==="));
	MojTestErrCheck(err);
	err = testBase64Err(_T("FPu-Aw=="));
	MojTestErrCheck(err);

	// base64 MIME
	err = testBase64Mime(NULL, 0, _T(""));
	MojTestErrCheck(err);
	MojByte input4[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e};
	err = testBase64Mime(input4, sizeof(input4), _T("FPucA9l+"));
	MojTestErrCheck(err);
	MojByte input5[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9};
	err = testBase64Mime(input5, sizeof(input5), _T("FPucA9k="));
	MojTestErrCheck(err);
	MojByte input6[] = {0x14, 0xfb, 0x9c, 0x03};
	err = testBase64Mime(input6, sizeof(input6), _T("FPucAw=="));
	MojTestErrCheck(err);
	err = testBase64MimeErr(_T("Qa6oL8==="));
	MojTestErrCheck(err);
	err = testBase64MimeErr(_T("Qa6oL==="));
	MojTestErrCheck(err);
	err = testBase64MimeErr(_T("Qa6-L8=="));
	MojTestErrCheck(err);

	// binary search
	MojTestAssert(MojBinarySearch(7, (int*) NULL, 0) == MojInvalidIndex);
	int array1[] = {8};
	MojTestAssert(MojBinarySearch(7, array1, 1) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(9, array1, 1) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(8, array1, 1) == 0);
	int array2[] = {2,24};
	MojTestAssert(MojBinarySearch(0, array2, 2) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(10, array2, 2) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(50, array2, 2) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(2, array2, 2) == 0);
	MojTestAssert(MojBinarySearch(24, array2, 2) == 1);
	int array3[] = {2,24,87};
	MojTestAssert(MojBinarySearch(0, array3, 3) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(10, array3, 3) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(50, array3, 3) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(100, array3, 3) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(2, array3, 3) == 0);
	MojTestAssert(MojBinarySearch(24, array3, 3) == 1);
	MojTestAssert(MojBinarySearch(87, array3, 3) == 2);
	int array4[] = {0,1,2,3,4,5,6,7,8,9};
	for (int i = 0; i < 10; ++i) {
		MojTestAssert(MojBinarySearch(i, array4, 10) == (MojSize) i);
	}
	const MojChar* array5[] = {_T("a"),_T("ab"),_T("b"),_T("baa")};
	MojTestAssert(MojBinarySearch(_T("hello"), array5, 4) == MojInvalidIndex);
	MojTestAssert(MojBinarySearch(_T("ab"), array5, 4) == 1);

	// quicksort
	MojQuickSort((int*)NULL, 0);
	int qs1[] = {5};
	int qs1Expected[] = {5};
	MojQuickSort(qs1, 1);
	MojTestAssert(MojMemCmp(qs1, qs1Expected, 1 * sizeof(int)) == 0);
	int qs2[] = {1,2};
	int qs2Expected[] = {1,2};
	MojQuickSort(qs2, 2);
	MojTestAssert(MojMemCmp(qs2, qs2Expected, 2 * sizeof(int)) == 0);
	int qs3[] = {2,1};
	int qs3Expected[] = {1,2};
	MojQuickSort(qs3, 2);
	MojTestAssert(MojMemCmp(qs3, qs3Expected, 2 * sizeof(int)) == 0);
	int qs4[] = {1,2,3};
	int qs4Expected[] = {1,2,3};
	MojQuickSort(qs4, 3);
	MojTestAssert(MojMemCmp(qs4, qs4Expected, 3 * sizeof(int)) == 0);
	int qs5[] = {3,1,2};
	int qs5Expected[] = {1,2,3};
	MojQuickSort(qs5, 3);
	MojTestAssert(MojMemCmp(qs5, qs5Expected, 3 * sizeof(int)) == 0);
	int qs6[] = {9,8,7,6,5,4,3,2,1,0};
	int qs6Expected[] = {0,1,2,3,4,5,6,7,8,9};
	MojQuickSort(qs6, 10);
	MojTestAssert(MojMemCmp(qs6, qs6Expected, 10 * sizeof(int)) == 0);

	// file to string
	MojString str;
	err = writeFile(TestFileName, _T(""));
	MojTestErrCheck(err);
	err = str.assign(_T("hello"));
	MojTestErrCheck(err);
	err = MojFileToString(TestFileName, str);
	MojTestErrCheck(err);
	MojTestAssert(str.empty());
	err = writeFile(TestFileName, _T("howdy"));
	MojTestErrCheck(err);
	err = MojFileToString(TestFileName, str);
	MojTestErrCheck(err);
	MojTestAssert(str == _T("howdy"));
	err = MojFileToString(_T("this file should not exist"), str);
	MojTestErrExpected(err, MojErrNotFound);

	return MojErrNone;
}

/**
***************************************************************************************************
* @testBase64             The testBase64() tests the MojBase64Encode encoding and the
                          MojBase64Decode decoding functionality.

* @param                : MojByte*,MojSize and the MojChar*
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::testBase64(const MojByte* src, MojSize size, const MojChar* expected)
{
	// encode
	MojString str;
	MojErr err = str.resize(MojBase64EncodedLenMax(size));
	MojTestErrCheck(err);
	MojString::Iterator iter;
	err = str.begin(iter);
	MojTestErrCheck(err);
	MojSize sizeOut;
	err = MojBase64Encode(src, size, iter, str.length(), sizeOut);
	MojTestErrCheck(err);
	MojTestAssert(str == expected);
	MojTestAssert(sizeOut == MojStrLen(expected));
	// decode
	MojVector<MojByte> vec;
	err = vec.resize(MojBase64DecodedSizeMax(str.length()));
	MojTestErrCheck(err);
	MojVector<MojByte>::Iterator vi;
	err = vec.begin(vi);
	MojTestErrCheck(err);
	err = MojBase64Decode(str.begin(), str.length(), vi, vec.size(), sizeOut);
	MojTestErrCheck(err);
	MojTestAssert(sizeOut == size && MojMemCmp(src, vi, size) == 0);

	return MojErrNone;
}

/**
***************************************************************************************************
* @testBase64Mime         The testBase64Mime() tests the MojBase64EncodeMIME encoding and the
                          MojBase64DecodeMIME decoding functionality.

* @param                : MojByte*,MojSize and the MojChar*
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::testBase64Mime(const MojByte* src, MojSize size, const MojChar* expected)
{
	// encode
	MojString str;
	MojErr err = str.resize(MojBase64EncodedLenMax(size));
	MojTestErrCheck(err);
	MojString::Iterator iter;
	err = str.begin(iter);
	MojTestErrCheck(err);
	MojSize sizeOut;
	err = MojBase64EncodeMIME(src, size, iter, str.length(), sizeOut);
	MojTestErrCheck(err);
	MojTestAssert(str == expected);
	MojTestAssert(sizeOut == MojStrLen(expected));
	// decode
	MojVector<MojByte> vec;
	err = vec.resize(MojBase64DecodedSizeMax(str.length()));
	MojTestErrCheck(err);
	MojVector<MojByte>::Iterator vi;
	err = vec.begin(vi);
	MojTestErrCheck(err);
	err = MojBase64DecodeMIME(str.begin(), str.length(), vi, vec.size(), sizeOut);
	MojTestErrCheck(err);
	MojTestAssert(sizeOut == size && MojMemCmp(src, vi, size) == 0);

	return MojErrNone;
}

/**
***************************************************************************************************
* @testBase64Err          The testBase64Err() tests the MojBase64Decode Decoding by giving
                          invalid data

* @param                : MojChar*
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::testBase64Err(const MojChar* str)
{
	MojVector<MojByte> vec;
	MojSize len = MojStrLen(str);
	MojErr err = vec.resize(MojBase64DecodedSizeMax(len));
	MojTestErrCheck(err);
	MojVector<MojByte>::Iterator vi;
	err = vec.begin(vi);
	MojTestErrCheck(err);
	MojSize sizeOut;
	err = MojBase64Decode(str, len, vi, vec.size(), sizeOut);
	MojTestErrExpected(err, MojErrInvalidBase64Data);

	return MojErrNone;
}

/**
***************************************************************************************************
* @testBase64MimeErr      The testBase64MimeErr() tests the MojBase64DecodeMime Decoding by giving
                          invalid data

* @param                : MojChar*
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::testBase64MimeErr(const MojChar* str)
{
	MojVector<MojByte> vec;
	MojSize len = MojStrLen(str);
	MojErr err = vec.resize(MojBase64DecodedSizeMax(len));
	MojTestErrCheck(err);
	MojVector<MojByte>::Iterator vi;
	err = vec.begin(vi);
	MojTestErrCheck(err);
	MojSize sizeOut;
	err = MojBase64DecodeMIME(str, len, vi, vec.size(), sizeOut);
	MojTestErrExpected(err, MojErrInvalidBase64Data);

	return MojErrNone;
}

/**
***************************************************************************************************
* @writeFile              The writeFile function opens the file and writes the data to the given file path.

* @param                : MojChar* and MojChar*
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojUtilTest::writeFile(const MojChar* path, const MojChar* data)
{
	(void) MojUnlink(path);

	MojFileT file = MojInvalidFile;
	MojErr err = MojFileOpen(file, path, MOJ_O_WRONLY | MOJ_O_CREAT, MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojErrCheck(err);
	MojSize len = MojStrLen(data);
	while (len > 0) {
		MojSize written = 0;
		err = MojFileWrite(file, data, len, written);
		MojErrGoto(err, Done);
		len -= written;
	}
Done:
	err = MojFileClose(file);
	MojErrCheck(err);

	return MojErrNone;
}

/**
***************************************************************************************************
* @cleanup                The functionality of cleanup is to unlink the filename and to remove the
                          created directory which was created for testing.
* @param                : None
* @retval               : None
***************************************************************************************************
**/
void MojUtilTest::cleanup()
{
	(void) MojRmDirRecursive(_T("foo"));
	(void) MojUnlink(TestFileName);
}
