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


#ifndef MOJDBSTORAGEENGINE_H_
#define MOJDBSTORAGEENGINE_H_

#include "db/MojDbDefs.h"
#include "db/MojDbWatcher.h"
#include "db/MojDbQuotaEngine.h"
#include "core/MojAutoPtr.h"
#include "core/MojObject.h"
#include "core/MojVector.h"

#include "db/MojDbStorageCollection.h"

class MojDbStorageIndex : public MojDbStorageCollection
{
public:
	typedef MojVector<MojByte> ByteVec;

	virtual ~MojDbStorageIndex() {}
	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
};

class MojDbStorageDatabase : public MojDbStorageCollection
{
public:
	virtual ~MojDbStorageDatabase() {}
	virtual MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn) = 0;
	virtual MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut) = 0;
	virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut) = 0;
	virtual MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut) = 0;
//hack:
	virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutexes_regionsize)
		{ if (total_mutexes) *total_mutexes = 0;
		  if (mutexes_free) *mutexes_free = 0;
		  if (mutexes_used) *mutexes_used = 0;
		  if (mutexes_used_highwater) *mutexes_used_highwater = 0;
		  if (mutexes_regionsize) *mutexes_regionsize = 0;
		  return MojErrNone;
		 }
};

class MojDbStorageEngineFactory : public MojRefCounted
{
public:
    virtual ~MojDbStorageEngineFactory() {}
    virtual MojErr create(MojRefCountedPtr<MojDbStorageEngine>& engineOut) const = 0;
    virtual MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) const = 0;
    virtual const MojChar* name() const = 0;
};

class MojDbStorageEngine : public MojRefCounted
{
public:
    static MojErr createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut);
    static MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);
    static MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) { return m_factory->createEnv(envOut); };
    static MojErr setEngineFactory(MojDbStorageEngineFactory* factory);
    static const MojDbStorageEngineFactory* engineFactory() {return m_factory.get();};

    virtual ~MojDbStorageEngine() {}
    virtual MojErr configure(const MojObject& config) = 0;
    virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn) = 0;
    virtual MojErr open(const MojChar* path) = 0;
    virtual MojErr open(const MojChar* path, MojDbEnv * env) = 0;
    virtual MojErr close() = 0;
    virtual MojErr compact() = 0;
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut) = 0;
    virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut) = 0;
    virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn,  MojRefCountedPtr<MojDbStorageSeq>& seqOut) = 0;

protected:
	MojDbStorageEngine();
	static MojRefCountedPtr<MojDbStorageEngineFactory> m_factory;

};


#endif /* MOJDBSTORAGEENGINE_H_ */
