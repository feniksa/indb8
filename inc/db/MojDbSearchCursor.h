/* @@@LICENSE
*
*  Copyright (c) 2009-2013 LG Electronics, Inc.
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


#ifndef MOJDBSEARCHCURSOR_H_
#define MOJDBSEARCHCURSOR_H_

#include "db/MojDbDefs.h"
#include "db/MojDbCursor.h"
#include "db/MojDbObjectItem.h"

class MojDbSearchCursor : public MojDbCursor
{
public:
	MojDbSearchCursor(MojString locale);
	virtual ~MojDbSearchCursor();

	MojErr close() override;
	MojErr get(MojDbStorageItem*& itemOut, bool& foundOut) override;
	MojErr count(MojUInt32& countOut) override;
	MojErr nextPage(MojDbQuery::Page& pageOut) override;

	MojErr setPagePosition();
	MojDbCollationStrength collation() const { return m_collation; }

private:
	struct ItemComp
	{
		int operator()(const MojRefCountedPtr<MojDbObjectItem>& i1, const MojRefCountedPtr<MojDbObjectItem>& i2) const
		{
			return i1->sortKeys().compare(i2->sortKeys());
		}
	};
	typedef MojSet<MojDbKey> KeySet;
	typedef MojSet<MojObject> ObjectSet;
	typedef MojMap<MojUInt32, MojSharedPtr<ObjectSet> > GroupMap;
	typedef MojVector<MojRefCountedPtr<MojDbObjectItem>, MojEq<MojRefCountedPtr<MojDbObjectItem> >, ItemComp > ItemVec;

	static const MojUInt32 MaxResults = 10000;

	MojErr init(const MojDbQuery& query) override;
    MojErr retrieveCollation(const MojDbQuery& query);
	bool loaded() const { return m_pos != NULL; }
	MojErr begin();
	MojErr load();
	MojErr loadIds(ObjectSet& idsOut);
	MojErr loadObjects(const ObjectSet& ids);
	MojErr sort();
	MojErr distinct();
    const MojDbQuery::Page& page() const { return m_page; }

	ItemVec m_items;
	MojString m_orderProp;
	MojString m_distinct;
	MojUInt32 m_limit;
	ItemVec::ConstIterator m_pos;
	ItemVec::ConstIterator m_limitPos;
	MojString m_locale;
    MojDbCollationStrength m_collation;
    MojDbQuery::Page m_page;
    MojUInt32 m_count;
};

#endif /* MOJDBSEARCHCURSOR_H_ */
