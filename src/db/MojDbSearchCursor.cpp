/* @@@LICENSE
*
*  Copyright (c) 2009-2015 LG Electronics, Inc.
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


#include "db/MojDbSearchCursor.h"
#include "db/MojDbExtractor.h"
#include "db/MojDbIndex.h"
#include "db/MojDbKind.h"
#include "db/MojDb.h"
#include "db/MojDbStorageQuery.h"
#include "db/MojDbPropExtractor.h"

MojDbSearchCursor::MojDbSearchCursor(MojString localeStr)
: m_limit(0),
  m_pos(NULL),
  m_locale(localeStr)
{
}

MojDbSearchCursor::~MojDbSearchCursor()
{
}

MojErr MojDbSearchCursor::close()
{
	MojErr err = MojErrNone;
	MojErr errClose = MojDbCursor::close();
	MojErrAccumulate(err, errClose);

	m_limit = 0;
	m_pos = NULL;
	m_limitPos = NULL;
	m_items.clear();

	return err;
}

MojErr MojDbSearchCursor::get(MojDbStorageItem*& itemOut, bool& foundOut)
{
	foundOut = false;
	MojErr err = begin();
	MojErrCheck(err);

	if (m_pos != m_limitPos) {
		foundOut = true;
		itemOut = m_pos->get();
		++m_pos;
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::count(MojUInt32& countOut)
{
	countOut = 0;
	MojErr err = begin();
	MojErrCheck(err);
    countOut = m_count;

	return MojErrNone;
}

/***********************************************************************
 * setPagePosition
 *
 * Move cursor position to page id provided in query.
 *   1. Find item "_id" which is equal to page id iteratively.
 *   2. If found, set begin/last position and next page.
 ***********************************************************************/
MojErr MojDbSearchCursor::setPagePosition()
{
    MojErr err;
    MojDbStorageItem* item;
    m_pos = m_items.begin();
    ItemVec::ConstIterator last = m_items.end();
    m_limitPos = last;
    // retrieve page id from query
    MojObject pageKey;
    err = m_page.toObject(pageKey);
    MojErrCheck(err);
    while (m_pos != last) {
        // retrieve id from query
        item = m_pos->get();
        const MojObject id = item->id();
        // If match, set begin/last position and next page
        if(pageKey.compare(id) == 0) {
            if (m_limit >= (last-m_pos)) {
                m_limitPos = m_items.end();
                m_page.clear();
            } else {
                m_limitPos = m_pos + m_limit;
                // set next page
                MojDbStorageItem* nextItem = m_limitPos->get();
                const MojObject nextId = nextItem->id();
                m_page.fromObject(nextId);
                // print log for nextId
                MojString strOut;
                err = nextId.toJson(strOut);
                MojErrCheck(err);
                MojLogDebug(MojDb::s_log, _T("nextId : %s \n"), strOut.data());
            }
            break;
        }
        ++m_pos;
    }

    return MojErrNone;
}

/***********************************************************************
 * nextPage
 *
 * Set next page if m_page is not null
 ***********************************************************************/
MojErr MojDbSearchCursor::nextPage(MojDbQuery::Page& pageOut)
{
    pageOut = m_page;

    return MojErrNone;
}

MojErr MojDbSearchCursor::init(const MojDbQuery& query)
{
	MojErr err = initImpl(query);
	MojErrCheck(err);

    err = retrieveCollation(query);
    MojErrCheck(err);

	// override limit and sort since we need to retrieve everything
	// and sort before re-imposing limit
	m_limit = query.limit();
	m_distinct = query.distinct();
	if (!m_distinct.empty()) {
		m_orderProp = m_distinct;
	} else {
		m_orderProp = query.order();
	}
    // retrieve page info from query.
    MojDbQuery::Page page;
    page = m_query.page();
    if(!page.empty()) {
        MojObject objOut;
        err = page.toObject(objOut);
        MojErrCheck(err);
        m_page.fromObject(objOut);
    }
	m_query.limit(MaxResults);
	err = m_query.order(_T(""));
	MojErrCheck(err);
    // delete page info from query for query plan
    page.clear();
    m_query.page(page);

	return MojErrNone;
}

MojErr MojDbSearchCursor::retrieveCollation(const MojDbQuery& query)
{
    m_collation = MojDbCollationInvalid;

    const MojChar* orderName = query.order().data();
    if (m_kindEngine && !query.order().empty()) {
        // Get index that is set by orderBy
        MojDbKind* kind = NULL;
        MojErr err = m_kindEngine->getKind(query.from().data(), kind);
        MojErrCheck(err);
        MojDbIndex* index = kind->indexForCollation(query);
        if(index != NULL) {
            // Find property name that is equal to orderBy name form index to retrieve collation strenth.
            MojDbIndex::StringVec propNames = index->props();
            for (MojSize idx = 0; idx < propNames.size(); idx++) {
                if(!(propNames.at(idx)).compare(orderName)) {
                    m_collation = index->collation(idx);
                    break;
                }
            }
        }
    }

    return MojErrNone;
}

MojErr MojDbSearchCursor::begin()
{
	if (!loaded()) {
		MojErr err = load();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::load()
{
	// pull unique ids from index
	ObjectSet ids;
	MojErr err = loadIds(ids);
	MojErrCheck(err);

	// load objects into memory
	err = loadObjects(ids);
	MojErrCheck(err);
	// sort results
	if (!m_orderProp.empty()) {
		err = sort();
		MojErrCheck(err);
	}
	// distinct
	if (!m_distinct.empty()) {
		distinct();
	}
	// reverse for desc
	if (m_query.desc()) {
		err = m_items.reverse();
		MojErrCheck(err);
	}
    // next page
    if (!m_page.empty()) {
        err = setPagePosition();
        MojErrCheck(err);
    } else {
        // set begin/last position.
        m_pos = m_items.begin();
        if (m_limit >= m_items.size()) {
            m_limitPos = m_items.end();
        } else {
            // if item size is bigger than limit, set next page.
            m_limitPos = m_items.begin() + m_limit;
            MojDbStorageItem* nextItem = m_limitPos->get();
            const MojObject nextId = nextItem->id();
            m_page.fromObject(nextId);
        }
    }
    // set remainder count
    m_count = m_items.end() - m_pos;

	return MojErrNone;
}

MojErr MojDbSearchCursor::loadIds(ObjectSet& idsOut)
{
	MojUInt32 groupNum = 0;
	bool found = false;
	MojSharedPtr<ObjectSet> group;
	GroupMap groupMap;

	for(;;) {
		// get current id
		MojObject id;
		MojUInt32 idGroupNum = 0;
		MojErr err = m_storageQuery->getId(id, idGroupNum, found);
		MojErrCheck(err);
		if (!found)
			break;

		// if it is in a new group, create a new set
		if (!group.get() || idGroupNum != groupNum) {
			// find/create new group
			GroupMap::Iterator iter;
			err = groupMap.find(idGroupNum, iter);
			MojErrCheck(err);
			if (iter != groupMap.end()) {
				group = iter.value();
			} else {
				err = group.resetChecked(new ObjectSet);
				MojErrCheck(err);
				err = groupMap.put(idGroupNum, group);
				MojErrCheck(err);
			}
			groupNum = idGroupNum;
		}
		// add id to current set
		err = group->put(id);
		MojErrCheck(err);
	}

	// no matches unless all groups are accounted for
	MojUInt32 groupCount = m_storageQuery->groupCount();
	for (MojUInt32 i = 0; i < groupCount; ++i) {
		if (!groupMap.contains(i))
			return MojErrNone;
	}

	// find intersection of all groups
	GroupMap::ConstIterator begin = groupMap.begin();
	for (GroupMap::ConstIterator i = begin; i != groupMap.end(); ++i) {
		if (i == begin) {
			// special handling for first group
			idsOut = *(i.value());
		} else {
			MojErr err = idsOut.intersect(*(i.value()));
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::loadObjects(const ObjectSet& ids)
{
	MojInt32 warns = 0;
	for (ObjectSet::ConstIterator i = ids.begin(); i != ids.end(); ++i) {
		// get item by id
		MojObject obj;
		MojDbStorageItem* item = NULL;
		bool found = false;
		MojErr err = m_storageQuery->getById(*i, item, found);
		if (err == MojErrInternalIndexOnFind) {
			warns++;
			continue;
		}

		MojErrCheck(err);
		if (found) {
			// get object from item
			err = item->toObject(obj, *m_kindEngine);
			MojErrCheck(err);
			// filter results
			if (m_queryFilter.get() && !m_queryFilter->test(obj))
				continue;
			// create object item
			MojRefCountedPtr<MojDbObjectItem> item(new MojDbObjectItem(obj));
			MojAllocCheck(item.get());
			// add to vec
			err = m_items.push(item);
			MojErrCheck(err);
		}
	}
	if (warns > 0)
        MojLogDebug(MojDb::s_log, _T("Search warnings: %d \n"), warns);
	return MojErrNone;
}

MojErr MojDbSearchCursor::sort()
{
	MojAssert(!m_orderProp.empty());

	// TODO: instead of parsing all objects, find the serialized field in the object and compare it directly
	// create extractor for sort prop
	MojRefCountedPtr<MojDbTextCollator> collator(new MojDbTextCollator);
	MojAllocCheck(collator.get());
	// TODO: use real locale
	//MojErr err = collator->init(_T(""), MojDbCollationPrimary);
    MojErr err = MojErrNone;

    // set locale
    MojString locale = m_locale;
    if(m_dbIndex) {
        locale = m_dbIndex->locale();
    }
    // set collate
    MojDbCollationStrength coll = m_collation;
    if (coll == MojDbCollationInvalid) {
        // default setting is primary
        coll = MojDbCollationPrimary;
    }
    err = collator->init(locale, coll);
    MojErrCheck(err);

	MojDbPropExtractor extractor;
	extractor.collator(collator.get());
	err = extractor.prop(m_orderProp);
	MojErrCheck(err);

	// create sort keys
	MojDbKeyBuilder builder;
	ItemVec::Iterator begin;
	err = m_items.begin(begin);
	MojErrCheck(err);
	for (ItemVec::Iterator i = begin; i != m_items.end(); ++i) {
		KeySet keys;
		err = extractor.vals((*i)->obj(), keys);
		MojErrCheck(err);
		(*i)->sortKeys(keys);
	}

	// sort
	err = m_items.sort();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbSearchCursor::distinct()
{
	MojAssert(!m_items.empty());

	ItemComp itemComp;
	MojSize idx = 0;
	MojSize itemSize = m_items.size();
	if (itemSize == 0)
		return MojErrNone;
	while(idx < itemSize-1) {
		if(itemComp(m_items[idx],m_items[idx+1]) == 0) {
			m_items.erase(idx+1);
			itemSize = m_items.size();
		} else {
			idx++;
		}
	}

	return MojErrNone;
}

