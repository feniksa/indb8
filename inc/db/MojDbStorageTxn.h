#pragma once

#include "core/MojSignal.h"
#include "db/MojDbDefs.h"
#include "db/MojDbQuotaEngine.h"
#include "db/MojDbKey.h"
#include "db/MojDbWatcher.h"

class MojDbStorageTxn : public MojSignalHandler
{
public:
	typedef MojVector<MojByte> ByteVec;
	typedef MojSignal<MojDbStorageTxn*> CommitSignal;

	virtual ~MojDbStorageTxn() {}
	virtual MojErr abort() = 0;
	virtual bool isValid() = 0;
	MojErr commit();

	MojErr addWatcher(MojDbWatcher* watcher, const MojDbKey& key);
	MojErr offsetQuota(MojInt64 amount);
	void quotaEnabled(bool val) { m_quotaEnabled = val; }
	void refreshQuotas() { m_refreshQuotas = true; }

	void notifyPreCommit(CommitSignal::SlotRef slot);
	void notifyPostCommit(CommitSignal::SlotRef slot);

protected:
	MojDbStorageTxn();
	virtual MojErr commitImpl() = 0;

private:
	friend class MojDbQuotaEngine;

	struct WatcherInfo
	{
		WatcherInfo(MojDbWatcher* watcher, const MojDbKey& key)
		: m_watcher(watcher), m_key(key) {}

		MojRefCountedPtr<MojDbWatcher> m_watcher;
		MojDbKey m_key;
	};
	typedef MojVector<WatcherInfo> WatcherVec;

	bool m_quotaEnabled;
	bool m_refreshQuotas;
	MojDbQuotaEngine* m_quotaEngine;
	MojDbQuotaEngine::OffsetMap m_offsetMap;
	MojRefCountedPtr<MojDbQuotaEngine::Offset> m_curQuotaOffset;
	WatcherVec m_watchers;
	CommitSignal m_preCommit;
	CommitSignal m_postCommit;
};
