#include "db/MojDbStorageTxn.h"

MojDbStorageTxn::MojDbStorageTxn()
: m_quotaEnabled(true),
  m_refreshQuotas(false),
  m_quotaEngine(NULL),
  m_preCommit(this),
  m_postCommit(this)
{
}

MojErr MojDbStorageTxn::addWatcher(MojDbWatcher* watcher, const MojDbKey& key)
{
	WatcherVec::Iterator i;
	MojErr err = m_watchers.begin(i);
	MojErrCheck(err);
	// check to see if we already have this watcher in our vec, and if so update it's minKey
	for (; i != m_watchers.end(); ++i) {
		if (i->m_watcher.get() == watcher) {
			if (key < i->m_key) {
				i->m_key = key;
			}
			break;
		}
	}
	if (i == m_watchers.end()) {
		MojErr err = m_watchers.push(WatcherInfo(watcher, key));
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStorageTxn::offsetQuota(MojInt64 offset)
{
	if (m_curQuotaOffset.get() && m_quotaEnabled) {
		MojErr err = m_curQuotaOffset->apply(offset);
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojDbStorageTxn::notifyPreCommit(CommitSignal::SlotRef slot)
{
	m_preCommit.connect(slot);
}

void MojDbStorageTxn::notifyPostCommit(CommitSignal::SlotRef slot)
{
	m_postCommit.connect(slot);
}

MojErr MojDbStorageTxn::commit()
{
	MojErr err = m_preCommit.fire(this);
	MojErrCheck(err);
	if (m_quotaEngine) {
		err = m_quotaEngine->applyUsage(this);
		MojErrCheck(err);
	}

	err = commitImpl();
	MojErrCheck(err);

	err = m_postCommit.fire(this);
	MojErrCheck(err);
	if (m_quotaEngine) {
		err = m_quotaEngine->applyQuota(this);
		MojErrCheck(err);
		if (m_refreshQuotas) {
			err = m_quotaEngine->refresh();
			MojErrCheck(err);
		}
	}

	WatcherVec vec;
	m_watchers.swap(vec);
	for (WatcherVec::ConstIterator i = vec.begin(); i != vec.end(); ++i) {
		MojErr err = i->m_watcher->fire(i->m_key);
		MojErrCheck(err);
	}
	return MojErrNone;
}
