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
#include "core/MojAutoPtr.h"
#include "core/MojObject.h"
#include "core/MojVector.h"
#include "db/MojDbStorageEngineFactory.h"

class MojDbStorageEngine : public MojRefCounted
{
public:
	static MojErr createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut);
	static MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);
	static MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) { return m_factory->createEnv(envOut); };
	static MojErr setEngineFactory(MojDbStorageEngineFactory* factory);
	static const MojDbStorageEngineFactory* engineFactory() { return m_factory.get(); };

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
