#pragma once 

#include "db/ui_files.h"

class FilesViewModelEx_t : public FilesViewModel_t
{
public:
	FilesViewModelEx_t(Core_t &r)
		: FilesViewModel_t(r)
	{
	}
protected:
	virtual void populateChildren(elt_t &aParent)//myITEMID parent)
	{
		//elt_t &aParent(at(parent));
		assert(aParent.uChildrenIndex == 0);
		assert(aParent.iChildrenNum < 0);
		assert(aParent.uMyIndex != muRootIndex);
		//assert(isValid(parent));
		CFolderPtr pFolder0(aParent.pFolder);
		assert(pFolder0);
		assert(pFolder0->fileFolder());

		CoreEx_t &core(dynamic_cast<CoreEx_t &>(mrCore));
		//ProjectEx_t &proj(core.projx());
		//ProjectInfoEx_t PJ(proj);
		//const FilesMgr0_t &rFiles(proj.files());
		Dc_t *pDC(DcInfo_t::DcFromFolder(*pFolder0));

		if (!pFolder0->fileModule())
			pFolder0 = skipSingles(pFolder0);

		const FoldersMap &m(pFolder0->fileFolder()->children());
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			CFolderPtr pFolder(&(*i));
			//const MyString &s(pFolder->name());

			adcui::FolderTypeEnum folderType(adcui::FOLDERTYPE_UNK);
			adcui::FolderTypeEnum folderType2(adcui::FOLDERTYPE_UNK);
			if (!pFolder->fileFolder())//a file?
			{
				if (!pDC)
				{
					if (pFolder->fileId() == FILEID_TYPES)
						folderType = adcui::FOLDERTYPE_FILE_T;
					else if (pFolder->fileId() == FILEID_NAMES)
						folderType = adcui::FOLDERTYPE_FILE_N;
					else if (pFolder->fileId() == FILEID_EXPORTS)
						folderType = adcui::FOLDERTYPE_FILE_E;
					else if (pFolder->fileId() == FILEID_IMPORTS)
						folderType = adcui::FOLDERTYPE_FILE_I;
					//else if (pFolder->fileId() == FILEID_TEMPLATES)
						//folderType = adcui::FOLDERTYPE_FILE_TT;
				}
				else if ((folderType = pDC->IsViewFile(*pFolder)) == adcui::FOLDERTYPE_UNK)
				{
					DcInfo_t DI(*pDC);
					FileDef_t *pFileDef(pFolder->fileDef());
					if (pFileDef/* && !pFileDef->objects().empty()*/)
					{
						if (DI.IsDefinitionFile(pFolder))//header only files
							folderType2 = adcui::FOLDERTYPE_FILE_CPP;
						if (DI.IsDeclarationFile(pFolder))
							folderType = adcui::FOLDERTYPE_FILE_H;
						else if (folderType2 != adcui::FOLDERTYPE_UNK)
							std::swap(folderType, folderType2);//prevent a header from dumping
					}
				}
			}
			else
			{
				assert(pFolder->Parent());
				folderType = adcui::FOLDERTYPE_FOLDER;
			}

			if (folderType2 != adcui::FOLDERTYPE_UNK)
				eddElt(pFolder, folderType2, aParent);

			eddElt(pFolder, folderType, aParent);
		}

		if (aParent.iChildrenNum < 0)
			aParent.iChildrenNum = 0;//mark as no children
	}

	virtual MyString name(const elt_t &elt) const
	{
		MyString s(FilesViewModel_t::name(elt));
		if (s.endsWith(MODULE_SEP) || s.endsWith(FOLDER_SEP))
			s.chop(1);
		s.append(FolderType2Ext(elt.eType));
		return s;
	}
};

