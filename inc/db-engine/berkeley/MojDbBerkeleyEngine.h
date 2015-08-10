/* @@@LICENSE
*
* Copyright (c) 2009-2015 LG Electronics, Inc.
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


#ifndef MOJDBBERKELEYENGINE_H_
#define MOJDBBERKELEYENGINE_H_

#include "db.h"
#include "db/MojDbDefs.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include "core/MojThread.h"
/*#include "db/MojDbEnv.h"
#include "db/MojDbObjectHeader.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDbStorageDatabase.h"
#include "db/MojDbStorageIndex.h"
#include "db/MojDbStorageItem.h"
#include "db/MojDbStorageSeq.h"
#include "db/MojDbStorageTxn.h"
#include "core/MojBuffer.h"
#include "core/MojFile.h"*/

#include "db/MojDbStorageEngine.h"

class MojDbBerkeleyEngine : public MojDbStorageEngine
{
public:
	static MojLogger s_log;

	MojDbBerkeleyEngine();
	~MojDbBerkeleyEngine();

	virtual MojErr configure(const MojObject& conf);
	virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn);
	virtual MojErr open(const MojChar* path);
    virtual MojErr open(const MojChar* path, MojDbEnv* env);
	virtual MojErr close();
	virtual MojErr compact();
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
	virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut);
	virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut);

	const MojString& path() const { return m_path; }
	MojDbBerkeleyEnv* env() { return m_env.get(); }
	MojDbBerkeleyDatabase* indexDb() { return m_indexDb.get(); }
	MojErr addDatabase(MojDbBerkeleyDatabase* db);
	MojErr removeDatabase(MojDbBerkeleyDatabase* db);
	MojErr addSeq(MojDbBerkeleySeq* seq);
	MojErr removeSeq(MojDbBerkeleySeq* seq);

private:
	typedef MojVector<MojRefCountedPtr<MojDbBerkeleyDatabase> > DatabaseVec;
	typedef MojVector<MojRefCountedPtr<MojDbBerkeleySeq> > SequenceVec;

	MojRefCountedPtr<MojDbBerkeleyEnv> m_env;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_seqDb;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_indexDb;
	MojThreadMutex m_dbMutex;
	MojString m_path;
	DatabaseVec m_dbs;
	SequenceVec m_seqs;
	bool m_isOpen;
};




#endif /* MOJDBBERKELEYENGINE_H_ */
