#pragma once

//#include "core/MojCoreDefs.h"
#include "db/MojDbStorageSeq.h"
#include <db.h>

class MojDbBerkeleyDatabase;

class MojDbBerkeleySeq : public MojDbStorageSeq
{
public:
	MojDbBerkeleySeq() : m_db(NULL), m_seq(NULL) {}
	~MojDbBerkeleySeq();

	MojErr open(const MojChar* name, MojDbBerkeleyDatabase* db);
	virtual MojErr close();
	virtual MojErr get(MojInt64& valOut);

	DB_SEQUENCE* impl() { return m_seq; }

private:
	friend class MojDbBerkeleyEngine;

	MojErr closeImpl();

	MojDbBerkeleyDatabase* m_db;
	DB_SEQUENCE* m_seq;
};
