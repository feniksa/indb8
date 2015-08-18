#pragma once

#include "core/MojCoreDefs.h"
#include "db/MojDbStorageTxn.h"
#include "db-engine/berkeley/MojDbBerkeleyDefs.h"
#include <db.h>

class MojDbBerkeleyTxn : public MojDbStorageTxn
{
public:
	MojDbBerkeleyTxn();
	~MojDbBerkeleyTxn();

	MojErr begin(MojDbBerkeleyEngine* env);
	virtual MojErr abort();
	virtual bool isValid();

	DB_TXN* impl() { return m_txn; }
	MojDbBerkeleyEngine* engine() { return m_engine; }
	void didUpdate(MojSize size) { m_updateSize += size; }
	MojSize updateSize() const { return m_updateSize; }

private:
	virtual MojErr commitImpl();

	MojDbBerkeleyEngine* m_engine;
	DB_TXN* m_txn;
	MojSize m_updateSize;
};
