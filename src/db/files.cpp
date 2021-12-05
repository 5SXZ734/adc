#include "files.h"
#include <assert.h>
#include "proj.h"
#include "info_proj.h"
#include "ui_main.h"
#include "type_module.h"

#if(!NO_FILE_ID)
int Folder_t::sID = 0;
#endif

Folder_t::Folder_t()
	: Base(""),
	m_nFlags(0),
	m_nRefCount(0),
	mID(0),
	mpOwner(nullptr),
	//miBinary(nullptr),
	mp(nullptr)
{
#if(!NO_FILE_ID)
	mID = ++sID;//1-biased
#endif
}

Folder_t::Folder_t(const MyFolderKey &s)
	: Base(s),
	m_nFlags(0),
	m_nRefCount(0),
	mID(0),
	mpOwner(nullptr),
	//miBinary(nullptr),
	mp(nullptr)
{
#if(!NO_FILE_ID)
	mID = ++sID;//1-biased
#endif
CHECK(mID == 22)
STOP
}

Folder_t::~Folder_t()
{
	assert(!mp);
	//delete mpName;
}

int Folder_t::setDesc(const char *szDesc)
{
	mDesc.clear();
	if (isempty(szDesc))
		return 1;
	mDesc = szDesc;
	if (!(mDesc.length() < 1024 - 1))
		mDesc.resize(1024-1);
	return 1;
}

int Folder_t::getDesc(char *szDesc) const
{
	assert(szDesc);
	if (!mDesc.empty())
		return 0;
	strcpy(szDesc, mDesc.c_str());
	return (int)mDesc.length();
}

int Folder_t::setName(const MyFolderKey &s)
{
	if (_key() == s)
		return -1;//already
	overrideKey(s);
	return 1;
}

/*void Folder_t::getName(MyFolderKey &s) const
{
	if (name().empty())//nameless())
	{
		int nIndex = ID();//Index();
		if (nIndex < 0)
			s = "?";
		else
			s =  MyStringf("file_%d", nIndex);
	}
	else
		s = name().c_str();
}*/

bool Folder_t::isNamedByUser() const
{
	if (name().startsWith(NAMEDEFAULT_DIR))
		return 0;
	if (name().startsWith(NAMEDEFAULT_FILE))
		return 0;
	return 1;
}




//////////////////////////////////

MyString FilesMgr0_t::GetRootDirectory()
{
	return mpRootFolder->name();
}

/*bool FilesMgr0_t::IsFolderLessThan(Folder_t &rSelf, const MyFileName &name, bool bFolder)
{
	if (IsFolder(rSelf))
	{
		if (!bFolder)
			return true;
	}
	else if (bFolder)
		return false;
	if (rSelf.name() < name)
		return true;
	return false;
}*/

bool FilesMgr0_t::IsFolder(const Folder_t &r)
{
	return r.fileFolder() || r.fileModule();
/*	const Folder_t &f(r);
	if (r.HasChildren())
		return true;
	MyString s(f.theName());
	if (s.endsWith("\\") || s.endsWith("/"))
		return true;
	return false;*/
}

bool FilesMgr0_t::IsModule(const Folder_t &r)
{
	return r.fileModule() != nullptr;
	//const Folder0_t &f(r.data());
	//MyString s(f.theName());
	//if (s.endsWith(MODULE_SEP))
		//return true;
	//return false;
}


MyString FilesMgr0_t::realPath(CFolderPtr p, bool bWithModule)
{
	MyString s;
	while (!IsModule(*p))
	{
		//MyString q(p->name());
		//if (p->fileFolder())
			//q.append("/");
		s.insert(0, p->name());//q
		p = p->Parent();
	}
#ifdef WIN32
	//avoid resembling a real path
	s.replace(':', '_');
#endif
	if (bWithModule)
	{
		assert(p->name().endsWith(MODULE_SEP));
		s.insert(0, p->name());
		s.replace((unsigned)p->name().length() - 1, (unsigned)strlen(MODULE_SEP), FOLDER_SEP);
	}
	return s;
}

MyString FilesMgr0_t::relPath(CFolderPtr p, CFolderPtr from)
{
	MyString s;
	while (p->Parent() && p != from)
	{
		s.insert(0, p->name());//q
		p = p->Parent();
	}
	return s;
}

MyString FilesMgr0_t::path(CFolderPtr p)
{
	MyString s;
	do {
		s.insert(0, p->name());//q
		p = p->Parent();
	} while (p);
	return s;
}

bool FilesMgr0_t::empty() const
{
	return mpRootFolder->fileFolder()->empty();
}

FolderPtr FilesMgr0_t::FindFileByStem(MyString s)//by stem (dir+basename)
{
	if (!s.empty())
	{
		for (FilesMgr0_t::FolderIterator i(*this); i; i++)
		{
			CFolderRef rFolder(*i);
			if (relPath(&rFolder) == s)
				return (FolderPtr)&rFolder;
		}
	}
	return nullptr;
}

FolderPtr FilesMgr0_t::FindFileByPath(MyString s, adcui::FolderTypeEnum &iKind)
{
	iKind = adcui::FOLDERTYPE_UNK;

	size_t n(s.findRev('.'));

	if (s.endsWith(HEADER_EXT))
	{
		iKind = adcui::FOLDERTYPE_FILE_H;
		s.truncate(s.findRev('.'));
	}
	else if (s.endsWith(SOURCE_EXT))
	{
		iKind = adcui::FOLDERTYPE_FILE_CPP;
		s.truncate(s.findRev('.'));
	}
	else if (s.endsWith(SOURCE_C_EXT))
	{
		iKind = adcui::FOLDERTYPE_FILE_C;
		s.truncate(s.findRev('.'));
	}

	return FindFileByStem(s);//ProjectInfo_t::fixFileName(s, nullptr));
}

#if(0)
bool FilesMgr0_t::IsMineFile(Fi le_t * pFile)
{
	for (FileTree_t::LeafIterator it(mRootFolder); it; it++)
	{
		Folder_t &folder(*it.data());
		Folder_t &file(folder.data());
		if (&file == pFile)
			return true;
	}
/*	for (Folder_t::iterator it(mpRootFolder->begin()); it != mpRootFolder->end(); it++)
	{
		Folder_t &file(**it);
		if (&file == pFile)
			return true;
	}*/
	return false;
}
#endif

void FilesMgr0_t::fixPath(MyString &s)
{
	s.replace('<', '_');
	s.replace('>', '_');
	s.replace(':', '_');
	s.replace('"', '_');
	s.replace('?', '_');
	s.replace('*', '_');
	s.replace(' ', '_');
}

MyString FolderType2Ext(adcui::FolderTypeEnum eType)
{
	switch (eType)
	{
	case adcui::FOLDERTYPE_FILE_CPP: return SOURCE_EXT;
	case adcui::FOLDERTYPE_FILE_C: return SOURCE_C_EXT;
	case adcui::FOLDERTYPE_FILE_H: return HEADER_EXT;
	//case adcui::FOLDERTYPE_FILE_RC: return RESOURCE_EXT;
	//case adcui::FOLDERTYPE_FILE_T: return TYPES_EXT;
	default: break;
	}
	return "";
}

adcui::FolderTypeEnum Ext2FolderType(const MyString &s)
{
	if (s.endsWith(".exe" MODULE_SEP, false))//case insensitive
		return adcui::FOLDERTYPE_BINARY_EXE;
	if (s.endsWith(".dll" MODULE_SEP, false))
		return adcui::FOLDERTYPE_BINARY_DLL;
	return adcui::FOLDERTYPE_UNK;
}



