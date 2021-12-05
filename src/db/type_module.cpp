#include "type_module.h"

//////////////////////////////////////////////////////
// Module_t

Module_t::Module_t()
	: mModuleTag(0),
	mUnique(-1),//not set
	mpModuleFolder(nullptr),
	mpDataSource(nullptr)
{
}

Module_t::~Module_t()
{
#if(USE_SEG_RANGES)
	//assert(!mpRange);
	assert(isRangeMgrEmpty());
#endif
	assert(!mpDataSource);
	//rawBlock().unload();
}

/*const MyString &Module_t::title() const
{
	if (mSubTitle.empty())
		return Seg_t::title();
	return mSubTitle;
}*/

void Module_t::setSubTitle(const MyString &s)
{
//#ifdef WIN32//FolderMap is case-insensitive on Windows
	//mSubTitle = s.lower();
//#else
	mSubTitle = s;
//#endif
}

MyString Module_t::makeSubtitle(const char *module, const char *submodule)
{
	MyString s(module);
	if (submodule && submodule[0])
	{
		s.push_back('(');
		s.append(MyString(submodule).lower());
		s.push_back(')');
	}
	return s;
}

bool Module_t::isRangeMgrEmpty() const
{
	for (RangeSetMapCIt i(mRangeMgr.begin()); i != mRangeMgr.end(); i++)
		if ((*i)->typeSeg()->hasFields())
			return false;
	return true;
}

bool Module_t::addSubRange(TypePtr iRangeSet)
{
	return mRangeMgr.insert(iRangeSet).second;
}

MyString Module_t::rangeSetName(TypePtr iSeg) const
{
	MyString s("$RANGESET");
	int n(0);
	for (RangeSetMapCIt i(mRangeMgr.begin()); i != mRangeMgr.end(); ++i, n++)
		if (*i == iSeg)
		{
			if (!iSeg->typeSeg()->title().empty() && iSeg->typeSeg()->title() != "$RANGESET")
				return iSeg->typeSeg()->title();
			return s + MyStringf("%d", n);
		}
	return s;
}

