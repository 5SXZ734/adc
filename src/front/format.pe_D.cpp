#include <math.h>
#include "shared.h"
#include "format.pe.h"
#include "decode.pe.h"

using namespace adcwin;

#define HIBIT(t)	~(((t)-1) >> 1)

/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(IMAGE_RESOURCE_DIRECTORY_ROOT)
{
	DECLDATAPTR(unsigned, p);

	if (p[0] & HIBIT(unsigned))
		mr.declBField("NameOffset", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	else
		mr.declBField("ResourceType", mr.arrayOf(mr.enumOf(mr.type("IMAGE_RESOURCE_TYPE"), TYPEID_DWORD), 31));
	mr.declBField("NameIsString", mr.arrayOf(mr.type(TYPEID_DWORD), 1));//<31>

	if (p[1] & HIBIT(unsigned))
		mr.declBField("OffsetToDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	else
		mr.declBField("OffsetToDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	mr.declBField("DataIsDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 1));//<31>
}
END_DYNAMIC_TYPE(IMAGE_RESOURCE_DIRECTORY_ROOT);


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(IMAGE_RESOURCE_DIRECTORY_ENTRY)
{
	DECLDATAPTR(unsigned, p);

	if (p[0] & HIBIT(unsigned))
		mr.declBField("NameOffset", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	else
		mr.declBField("Id", mr.arrayOf(mr.type(TYPEID_DWORD), 31));//WORD
	mr.declBField("NameIsString", mr.arrayOf(mr.type(TYPEID_DWORD), 1));//<31>

	if (p[1] & HIBIT(unsigned))
		mr.declBField("OffsetToDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	else
		mr.declBField("OffsetToDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
	mr.declBField("DataIsDirectory", mr.arrayOf(mr.type(TYPEID_DWORD), 1));//<31>
}
END_DYNAMIC_TYPE(IMAGE_RESOURCE_DIRECTORY_ENTRY);


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(IMAGE_IMPORT_BY_NAME)
{
	mr.declField("Hint", mr.type(TYPEID_WORD));
	mr.declField("Name", toAscii(mr));
}
END_DYNAMIC_TYPE(IMAGE_IMPORT_BY_NAME);


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(NormalMenuItem)
{
	mr.declField("flags", mr.type(TYPEID_WORD));
	mr.declField("id", mr.type(TYPEID_WORD));
	mr.declField("text", toUnicode(mr), ATTR_UNICODE);
}
END_DYNAMIC_TYPE(NormalMenuItem);


////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(PopupMenuItem)
{
	mr.declField("flags", mr.type(TYPEID_WORD));
	mr.declField("text", toUnicode(mr), ATTR_UNICODE);
}
END_DYNAMIC_TYPE(PopupMenuItem);

/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(VersionString)
{
	//DECLDATA(WORD, wLenth);//whole structure
	mr.declField("wLength", mr.type(TYPEID_WORD));
	DECLDATA(WORD, wValueLength);
	mr.declField("wValueLength", mr.type(TYPEID_WORD));
	mr.declField("wType", mr.type(TYPEID_WORD));
	mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
	if (wValueLength > 0)
	{
		mr.align(ALIGN_DWORD);
		mr.declField("Value", toUnicode(mr), ATTR_UNICODE);
	}
}
END_DYNAMIC_TYPE(VersionString);

/////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(DLGTEMPLATE)
{
	DECLDATA(DWORD, dw);
	bool bDlgEx((dw >> 16) == 0xFFFF);
	WORD cdit;
	bool bSetFont;
	if (bDlgEx)
	{
		DECLDATAEX(DLGTEMPLATEEX, a);
		cdit = a.cDlgItems;
		bSetFont = (a.style & (DS_SETFONT | DS_FIXEDSYS)) != 0;
		mr.declField("dlgVer", mr.type(TYPEID_WORD));
		mr.declField("signature", mr.type(TYPEID_WORD));
		mr.declField("helpID", mr.type(TYPEID_DWORD));
		mr.declField("exStyle", mr.type(TYPEID_DWORD));
		mr.declField("style", mr.type(TYPEID_DWORD));
	}
	else
	{
		DECLDATAEX(DLGTEMPLATE, a);
		cdit = a.cdit;
		bSetFont = (a.style & DS_SETFONT) != 0;
		mr.declField("style", mr.type(TYPEID_DWORD));
		mr.declField("extendedStyle", mr.type(TYPEID_DWORD));
	}

	mr.declField("items", mr.type(TYPEID_WORD));
	mr.declField("x", mr.type(TYPEID_SHORT));
	mr.declField("y", mr.type(TYPEID_SHORT));
	mr.declField("cx", mr.type(TYPEID_SHORT));
	mr.declField("cy", mr.type(TYPEID_SHORT));

	//mr.declField("DlgTemplate", mr.type("DLGTEMPLATE"));

	/*Immediately following the DLGTEMPLATE structure is a menu array that identifies a menu resource for the dialog box.
	If the first element of this array is 0x0000, the dialog box has no menu and the array has no other elements.
	If the first element is 0xFFFF, the array has one additional element that specifies the ordinal value of a menu resource in an executable file.
	If the first element has any other value, the system treats the array as a null-terminated Unicode string that specifies the name of a menu resource in an executable file.*/
	DECLDATA(WORD, wMenu);
	mr.declField("Menu", toUnicode(mr), ATTR_UNICODE);
	if (wMenu == 0xFFFF)
		mr.declField("MenuId", mr.type(TYPEID_WORD));

	/*Following the menu array is a class array that identifies the window class of the control.
	If the first element of the array is 0x0000, the system uses the predefined dialog box class for the dialog box and the array has no other elements.
	If the first element is 0xFFFF, the array has one additional element that specifies the ordinal value of a predefined system window class.
	If the first element has any other value, the system treats the array as a null-terminated Unicode string that specifies the name of a registered window class.*/
	DECLDATA(WORD, wClass);
	mr.declField("Class", toUnicode(mr), ATTR_UNICODE);
	if (wClass == 0xFFFF)
		mr.declField("ClassId", mr.type(TYPEID_WORD));

	/*Following the class array is a title array that specifies a null-terminated Unicode string that contains the title of the dialog box.
	If the first element of this array is 0x0000, the dialog box has no title and the array has no other elements.*/
	mr.declField("Title", toUnicode(mr), ATTR_UNICODE);

	if (bSetFont)
	{
		/*The 16-bit point size value and the typeface array follow the title array, but only if the style member specifies the DS_SETFONT style.*/
		mr.declField("PointSize", mr.type(TYPEID_WORD));
		if (bDlgEx)
		{
			mr.declField("Weight", mr.type(TYPEID_WORD));
			mr.declField("Italic", mr.type(TYPEID_BYTE));
			mr.declField("Charset", mr.type(TYPEID_BYTE));
		}
		/*The typeface array is a null-terminated Unicode string specifying the name of the typeface for the font. */
		mr.declField("Typeface", toUnicode(mr), ATTR_UNICODE);
	}
	/*Following the DLGTEMPLATE header in a standard dialog box template are one or more DLGITEMTEMPLATE structures that define the dimensions
	and style of the controls in the dialog box. The cdit member specifies the number of DLGITEMTEMPLATE structures in the template.
	These DLGITEMTEMPLATE structures must be aligned on DWORD boundaries.*/
}
END_DYNAMIC_TYPE(DLGTEMPLATE);



//////////////////////////////////////////////////
class DynamicType_RT_DIALOG_ITEM : public I_DynamicType
{
	bool bDlgEx;
public:
	DynamicType_RT_DIALOG_ITEM(bool b) : bDlgEx(b){}
	virtual const char *name() const { return bDlgEx ? _PFX("DLGITEMTEMPLATEEX") : _PFX("DLGITEMTEMPLATE"); }
	virtual void createz(I_Module &mr, unsigned long)
	{
		if (bDlgEx)
			mr.declField("helpID", mr.type(TYPEID_DWORD));
		mr.declField("style", mr.type(TYPEID_DWORD));
		mr.declField("exStyle", mr.type(TYPEID_DWORD));
		mr.declField("x", mr.type(TYPEID_SHORT));
		mr.declField("y", mr.type(TYPEID_SHORT));
		mr.declField("cx", mr.type(TYPEID_SHORT));
		mr.declField("cy", mr.type(TYPEID_SHORT));
		if (bDlgEx)
			mr.declField("id", mr.type(TYPEID_DWORD));
		else
			mr.declField("id", mr.type(TYPEID_WORD));
		//mr.declField("DlgItemTemplate", mr.type("DLGITEMTEMPLATE"));

		/*Immediately following each DLGITEMTEMPLATE structure is a class array that specifies the window class of the control.
		If the first element of this array is any value other than 0xFFFF,
		the system treats the array as a null-terminated Unicode string that specifies the name of a registered window class.
		If the first element is 0xFFFF, the array has one additional element that specifies the ordinal value of a predefined system class.*/
		DECLDATA(WORD, wClass);
		if (wClass == 0xFFFF)
		{
			mr.declField("ClassIdTag", mr.type(TYPEID_WORD));
			mr.declField("ClassId", mr.type(TYPEID_WORD));
		}
		else
			mr.declField("Class", toUnicode(mr), ATTR_UNICODE);

		/*Following the class array is a title array that contains the initial text or resource identifier of the control.
		If the first element of this array is 0xFFFF, the array has one additional element that specifies an ordinal value of a resource,
		such as an icon, in an executable file. If the first element is any value other than 0xFFFF,
		the system treats the array as a null-terminated Unicode string that specifies the initial text.*/
		DECLDATA(WORD, wTitle);
		if (wTitle == 0xFFFF)
		{
			mr.declField("TitleIdTag", mr.type(TYPEID_WORD));
			mr.declField("TitleId", mr.type(TYPEID_WORD));
		}
		else
			mr.declField("Title", toUnicode(mr), ATTR_UNICODE);

		/*The creation data array begins at the next WORD boundary after the title array.
		This creation data can be of any size and format. If the first word of the creation data array is nonzero,
		it indicates the size, in bytes, of the creation data (including the size word). */
		DECLDATA(WORD, d);
		if (d != 0)
			mr.declField("Data", mr.arrayOf(mr.type(TYPEID_WORD), d));
		else
			mr.declField("NoData", mr.type(TYPEID_WORD));
	}
};

DECLARE_DYNAMIC_TYPE1(DynamicType_RT_DIALOG_ITEM, DLGITEMTEMPLATE, false);
DECLARE_DYNAMIC_TYPE1(DynamicType_RT_DIALOG_ITEM, DLGITEMTEMPLATEEX, true);




/////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(RES_DIALOG)
{
#if(0)
#define SHIFTBY 29
	mr.skip(SHIFTBY);
	mr.declField("overlapped", type(TYPEID_WORD));
	mr.skip(-int(SHIFTBY + sizeof(WORD)));
#endif
	DECLDATA(DWORD, dw);
	bool bDlgEx((dw >> 16) == 0xFFFF);
	if (bDlgEx)
	{
		DECLDATAEX(DLGTEMPLATEEX, dlg);
		mr.declField(nullptr, mr.type(_PFX("DLGTEMPLATE")));
		for (int i(0); i < dlg.cDlgItems; i++)
		{
			//DECLDATA(DLGITEMTEMPLATE, a2);
			mr.align(ALIGN_DWORD);
			mr.declField(nullptr, mr.type(_PFX("DLGITEMTEMPLATEEX")));
		}
	}
	else
	{
		DECLDATAEX(DLGTEMPLATE, dlg);
		mr.declField(nullptr, mr.type(_PFX("DLGTEMPLATE")));
		for (int i(0); i < dlg.cdit; i++)
		{
			//DECLDATA(DLGITEMTEMPLATE, a2);
			mr.align(ALIGN_DWORD);
			mr.declField(nullptr, mr.type(_PFX("DLGITEMTEMPLATE")));
		}
	}
}
END_DYNAMIC_TYPE(RES_DIALOG);



//////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(RES_ICON)
{
	mr.installNamespace();
	DECLDATAEX(BITMAPINFOHEADER, bmiHeader);

	if (bmiHeader.biSize > sizeof(BITMAPINFOHEADER))
		mr.error("Resource bitmap format not recognized");

	POSITION beg(mr.cp());
	mr.declField("BitmapInfoHeader", mr.type("BITMAPINFOHEADER"));

	// Define the color table
	if ((bmiHeader.biBitCount != 24) && (bmiHeader.biBitCount != 32))
	{
		if (bmiHeader.biClrUsed > 0)
			mr.declField("Pallete", mr.arrayOf(mr.type("RGBQUAD"), bmiHeader.biClrUsed));
		else
			mr.declField("Pallete", mr.arrayOf(mr.type("RGBQUAD"), 1 << bmiHeader.biBitCount));
	}

	if (bmiHeader.biCompression > 0)
	{
		// Bytes are compressed
		DWORD bytes(bmiHeader.biSizeImage);
		if (bytes == 0)
			bytes = bmiHeader.biSize - (mr.cp() - beg);
		mr.declField("rleData", mr.arrayOf(mr.type(TYPEID_BYTE), bytes));
		return;
	}

	// Calculate bytes per line and padding required
	int bytesPerLine = (int)ceil(bmiHeader.biWidth * bmiHeader.biBitCount / 8.0);

#if(1)
	int padding = 4 - (bytesPerLine % 4);
	if (padding == 4)
		padding = 0;
	if (bmiHeader.biBitCount <= 8)
	{
		int bytesPerLine2(bmiHeader.biBitCount < 8 ? bytesPerLine : bmiHeader.biWidth);
		int x((bytesPerLine2 + padding) / sizeof(DWORD));
		int y(bmiHeader.biHeight / 2);
		mr.declField("ImageData", mr.arrayOf(mr.arrayOf(mr.type(TYPEID_DWORD), x), y), ATTR_HEX);
	}
	else
#endif

		// Define color data
		if (mr.NewScope(mr.declField("ImageData")))
		{
			for (int i(0); i < bmiHeader.biHeight / 2; i++)
			{
				mr.align(ALIGN_DWORD);
				if (bmiHeader.biBitCount < 8)
					mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), bytesPerLine));//imageData
				else if (bmiHeader.biBitCount == 8)
					mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), bmiHeader.biWidth));//colorIndex
				else if (bmiHeader.biBitCount == 24)
					mr.declField(nullptr, mr.arrayOf(mr.type("RGBTRIPLE"), bmiHeader.biWidth));//colors
				else if (bmiHeader.biBitCount == 32)
					mr.declField(nullptr, mr.arrayOf(mr.type("RGBQUAD"), bmiHeader.biWidth));//colors
			}
			mr.Leave();
		}

	int n1(bmiHeader.biHeight / 2);
	int n2(((bmiHeader.biWidth + 31) / 32) * 4);

#if(0)
	// Define each line of the mask
	if (mr.NewScope(mr.declField("ImageMask")))
	{
		for (int i(0); i < n1; i++)
			mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), n2), ATTR_BINARY);
		mr.Leave();
	}
#else
	mr.declField("ImageMask", mr.arrayOf(mr.arrayOf(mr.type(TYPEID_BYTE), n2), n1), ATTR_BINARY);
#endif
}
END_DYNAMIC_TYPE(RES_ICON);




//////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(RES_BITMAP)
{
	mr.installNamespace();
	DECLDATAEX(BITMAPINFOHEADER, bmiHeader);

	if (bmiHeader.biSize > sizeof(BITMAPINFOHEADER))
		mr.error("Format not recognized");

	POSITION beg(mr.cp());
	mr.declField("BitmapInfoHeader", mr.type("BITMAPINFOHEADER"));

	// Define the color table
	if ((bmiHeader.biBitCount != 24) && (bmiHeader.biBitCount != 32))
	{
		int items(bmiHeader.biClrUsed > 0 ? bmiHeader.biClrUsed : 1 << bmiHeader.biBitCount);
		mr.declField("Pallete", mr.arrayOf(mr.type("RGBQUAD"), items));
	}

	if (bmiHeader.biCompression > 0)
	{
		// Bytes are compressed
		DWORD bytes(bmiHeader.biSizeImage);
		if (bytes == 0)
			bytes = bmiHeader.biSize - (mr.cp() - beg);
		mr.declField("rleData", mr.arrayOf(mr.type(TYPEID_BYTE), bytes));
		return;
	}

	// Calculate bytes per line and padding required
	int bytesPerLine = (int)ceil(bmiHeader.biWidth * bmiHeader.biBitCount / 8.0);

#if(1)
	int padding = 4 - (bytesPerLine % 4);
	if (padding == 4)
		padding = 0;
	if (bmiHeader.biBitCount <= 8)
	{
		int bytesPerLine2(bmiHeader.biBitCount < 8 ? bytesPerLine : bmiHeader.biWidth);
		int x((bytesPerLine2 + padding) / sizeof(DWORD));
		int y(bmiHeader.biHeight);
		mr.declField(nullptr, mr.arrayOf(mr.arrayOf(mr.type(TYPEID_DWORD), x), y));
	}
	else
#endif
		// Define color data
		if (mr.NewScope(mr.declField("ImageData")))
		{
			for (int i(0); i < bmiHeader.biHeight; i++)
			{
				mr.align(ALIGN_DWORD);
				if (bmiHeader.biBitCount < 8)
					mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), bytesPerLine));//imageData
				else if (bmiHeader.biBitCount == 8)
					mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), bmiHeader.biWidth));//colorIndex
				else if (bmiHeader.biBitCount == 24)
					mr.declField(nullptr, mr.arrayOf(mr.type("RGBTRIPLE"), bmiHeader.biWidth));//colors
				else if (bmiHeader.biBitCount == 32)
					mr.declField(nullptr, mr.arrayOf(mr.type("RGBQUAD"), bmiHeader.biWidth));//colors
			}
			mr.Leave();
		}
}
END_DYNAMIC_TYPE(RES_BITMAP);

///////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(VS_VERSIONINFO)
{
#if(1)
	DECLDATA(WORD, u0);
	mr.declField("wLength", mr.type(TYPEID_WORD));
	DECLDATA(WORD, wValueLength);
	mr.declField("wValueLength", mr.type(TYPEID_WORD));
	mr.declField("wType", mr.type(TYPEID_WORD));
	mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
	//Arbitrary data associated with this VS_VERSIONINFO structure.The wValueLength member specifies the length of this member;
	//if wValueLength is zero, this member does not exist.
	if (wValueLength != 0)
	{
		mr.align(ALIGN_DWORD);
		mr.declField("FixedFileInfo", mr.type("VS_FIXEDFILEINFO"));
	}
	//An array of zero or one StringFileInfo structures, and zero or one VarFileInfo structures that are children of the current VS_VERSIONINFO structure
	struct header_t
	{
		WORD wLength;
		WORD wValueLength;
		WORD wType;
		//wchar_t szKey[1];
	};
	for (int i(0); i < 2; i++)//StringFileInfo or VarFileInfo (can be in any order, or missing?)
	{
		mr.align(ALIGN_DWORD);
		//DECLDATAEXX(header_t, h);
		DataStream_t ds(mr, mr.cpr() + sizeof(header_t));

		if (ds.strCmp(u"StringFileInfo") == 0)
			//if (wcscmp(h.szKey, L"StringFileInfo") == 0)
		{
			if (mr.NewScope(mr.declField("StringFileInfo")))
			{
				SAFE_SCOPE_HERE(mr);
				//mr.installNamespace();
				DECLDATA(WORD, u1);
				mr.declField("wLength", mr.type(TYPEID_WORD));
				DECLDATA(WORD, u2);
				mr.declField("wValueLength", mr.type(TYPEID_WORD));
				mr.declField("wType", mr.type(TYPEID_WORD));
				mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
				//mr.Leave(); return;
				mr.align(ALIGN_DWORD);
				if (mr.NewScope(mr.declField("StringTable")))
				{
					SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					DECLDATA(WORD, wLength);//The length, in bytes, of this StringTable structure, including all structures indicated by the Children member.
					POSITION beg(mr.cp());
					mr.declField("wLength", mr.type(TYPEID_WORD));
					mr.declField("wValueLength", mr.type(TYPEID_WORD));
					mr.declField("wType", mr.type(TYPEID_WORD));
					mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
					//mr.align(ALIGN_DWORD);
					if (mr.NewScope(mr.declField("Children")))
					{
						SAFE_SCOPE_HERE(mr);
						for (;;)
						{
							mr.align(ALIGN_DWORD);
							POSITION cur(mr.cp());
							if (!(cur - beg < wLength))
								break;
							mr.declField("Child_#", mr.type(_PFX("VersionString")));
						}
//						mr.Leave();
					}
//					mr.Leave();
				}
//				mr.Leave();
			}
		}
		else if (ds.strCmp(u"VarFileInfo") == 0)
			//else if (wcscmp(h.szKey, L"VarFileInfo") == 0)
		{
			mr.align(ALIGN_DWORD);
			if (mr.NewScope(mr.declField("VarFileInfo")))
			{
				SAFE_SCOPE_HERE(mr);
				//mr.installNamespace();
				mr.declField("wLength", mr.type(TYPEID_WORD));
				mr.declField("wValueLength", mr.type(TYPEID_WORD));
				mr.declField("wType", mr.type(TYPEID_WORD));
				mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
				mr.align(ALIGN_DWORD);
				if (mr.NewScope(mr.declField("Var")))
				{
					SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					DECLDATA(WORD, wLength);//The length, in bytes, of the Var structure.
					POSITION beg(mr.cp());
					mr.declField("wLength", mr.type(TYPEID_WORD));
					mr.declField("wValueLength", mr.type(TYPEID_WORD));
					mr.declField("wType", mr.type(TYPEID_WORD));
					mr.declField("szKey", toUnicode(mr), ATTR_UNICODE);
					mr.align(ALIGN_DWORD);
					unsigned l(wLength - (mr.cp() - beg));
					mr.declField("Value", mr.arrayOf(mr.type(TYPEID_DWORD), l / sizeof(DWORD)));
//					mr.Leave();
				}
//				mr.Leave();
			}
		}
	}
#endif
}
END_DYNAMIC_TYPE(VS_VERSIONINFO);



//////////////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(RES_MENU)
{
	DECLDATAEX(MENUHEADER, a);
	//assert(a.wVersion == 0 && a.cbHeaderSize == 0);
	if (!(a.wVersion == 0 && a.cbHeaderSize == 0))
		mr.error("Bad resource menu header");
	mr.declField(nullptr, mr.type("MENUHEADER"));
	unsigned level(1);
	for (;;)
	{
		DECLDATA(WORD, b);
		if (b & MF_POPUP)
		{
			mr.declField(nullptr, mr.type(_PFX("PopupMenuItem")));
			level++;
		}
		else
			mr.declField(nullptr, mr.type(_PFX("NormalMenuItem")));
		if (b & MF_END)
			if (--level == 0)
				break;
	}
}
END_DYNAMIC_TYPE(RES_MENU)



/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(RES_STRINGS)
{
	for (int i(0); i < 16; i++)
	{
		DECLDATA(WORD, a);
		if (a != 0)
			mr.declField("String_#", toNUnicode(mr), ATTR_NUNICODE);
		else
			mr.skip(sizeof(WORD));
	}
}
END_DYNAMIC_TYPE(RES_STRINGS)





/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COR_MetaDataRoot)
{
	mr.declField("Signature", mr.type(TYPEID_DWORD));
	mr.declField("MajorVersion", mr.type(TYPEID_WORD));
	mr.declField("MinorVersion", mr.type(TYPEID_WORD));
	mr.declField("Reserved", mr.type(TYPEID_DWORD));
	DECLDATA(DWORD, uLength);
	mr.declField("Length", mr.type(TYPEID_DWORD));
	mr.declField("Version", mr.arrayOf(mr.type(TYPEID_CHAR), uLength), ATTR_UTF8);//must be UTF-8
	mr.align(ALIGN_DWORD);
	mr.declField("Flags", mr.type(TYPEID_WORD));
	DECLDATA(WORD, uStreams);
	mr.declField("Streams", mr.type(TYPEID_WORD));
	/*for (unsigned u(0); u < uStreams; u++)
	{
	mr.instField("StreamHeaders", mr.type(_PFX("COR_StreamHeader")));
	mr.align(ALIGN_DWORD);
	}*/
}
END_DYNAMIC_TYPE(COR_MetaDataRoot);

/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COR_StreamHeader)
{
	mr.declField("Offset", mr.type(TYPEID_DWORD), (AttrIdEnum)ATTR_NET_METADATA_OFFS);
	mr.declField("Size", mr.type(TYPEID_DWORD));
	mr.declField("Name", toAscii(mr));
}
END_DYNAMIC_TYPE(COR_StreamHeader);

/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COR_TablesRoot)
{
	mr.declField("Reserved", mr.type(TYPEID_DWORD));
	mr.declField("MajorVersion", mr.type(TYPEID_BYTE));
	mr.declField("MinorVersion", mr.type(TYPEID_BYTE));
	mr.declField("HeapSizes", mr.type(TYPEID_BYTE));
	//mr.declField("Reserved2", mr.type(TYPEID_BYTE));
	mr.align(ALIGN_DWORD);
	DECLDATA(QWORD, uValid);
	mr.declField("Valid", mr.type(TYPEID_QWORD), ATTR_BINARY);
	//DECLDATA(QWORD, uSorted);
	mr.declField("Sorted", mr.type(TYPEID_QWORD), ATTR_BINARY);
	unsigned n(CountBits((QWORD)uValid));
	mr.declField("Rows", mr.arrayOf(mr.type(TYPEID_DWORD), n), ATTR_DECIMAL);
}
END_DYNAMIC_TYPE(COR_TablesRoot);


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COR_Blob)
{
	unsigned uDataSize;
	DECLDATAPTR(BYTE, c);
	if ((c[0] >> 7) == 0)
	{
		uDataSize = c[0] & 0x7F;
		mr.declField("Size1", mr.type(TYPEID_BYTE), ATTR_DECIMAL);
	}
	else if ((c[0] >> 6) == 2)
	{
		uDataSize = ((c[0] & 0x3F) << 8) + c[1];
		mr.declField("Size2", mr.type(TYPEID_WORD), ATTR_DECIMAL);
	}
	else
	{
		mr.declField("Size4", mr.type(TYPEID_DWORD), ATTR_DECIMAL);
		assert((c[0] >> 5) == 6);
		uDataSize = ((c[0] & 0x1F) << 24) + (c[1] << 16) + (c[2] << 8) + c[3];
	}
	if (uDataSize > 0)
		mr.declField("Data", mr.arrayOf(mr.type(TYPEID_BYTE), uDataSize));
}
END_DYNAMIC_TYPE(COR_Blob);


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COR_UserString)
{
	DECLDATA(BYTE, c);
	mr.declField("Length", mr.type(TYPEID_BYTE), ATTR_DECIMAL);
	if (c > 0)
	{
		unsigned uLength(unsigned(c) / 2);
		if (uLength > 0)
			mr.declField("Data", mr.arrayOf(mr.type(TYPEID_WORD), uLength), ATTR_UNICODE);
		if (c & 1)//terminating byte
			mr.declField("Eos", mr.type(TYPEID_BYTE));
	}
}
END_DYNAMIC_TYPE(COR_UserString);




/////////////////////////////////////////////////////////////

BEGIN_DYNAMIC_TYPE(UNWIND_INFO)
{
	DECLDATAEX(UNWIND_INFO, uwi);

#if(0)//test
	mr.skipBits(1);
	mr.declBField("a1", mr.arrayOf(mr.type(TYPEID_BYTE), 2));
	mr.skipBits(1);
	mr.declBField("a2", mr.arrayOf(mr.type(TYPEID_BYTE), 3));//4
	//mr.skip(1);
#else
	mr.declBField("Version", mr.arrayOf(mr.type(TYPEID_BYTE), 3));//0
	mr.declBField("Flags", mr.arrayOf(mr.enumOf(mr.type("UNW_FLAG"), TYPEID_BYTE), 5), ATTR_BINARY);//4
#endif

#if(0)//test
	mr.skipBits(1);
	mr.declBField("b1", mr.arrayOf(mr.type(TYPEID_BYTE), 2));
	mr.skipBits(1);
	mr.declBField("b2", mr.arrayOf(mr.type(TYPEID_BYTE), 3));//4
#else
	mr.declField("SizeOfProlog", mr.type(TYPEID_BYTE));
#endif
	mr.declField("CountOfUnwindCodes", mr.type(TYPEID_BYTE));

	//mr.declField("FrameReg", mr.type(TYPEID_BYTE));
	mr.declBField("FrameReg", mr.arrayOf(mr.type(TYPEID_BYTE), 4));//<0>
	mr.declBField("FrameRegOffs", mr.arrayOf(mr.type(TYPEID_BYTE), 4));//<4>

	if (uwi.CountOfCodes > 0)
	{
		mr.declField("UnwindCodes", mr.arrayOf(mr.type("UNWIND_CODE"), uwi.CountOfCodes));
	}
#if(1)
	if (!(uwi.Flags & UNW_FLAG_CHAININFO))
	{
		if ((uwi.Flags & UNW_FLAG_EHANDLER) || (uwi.Flags & UNW_FLAG_UHANDLER))
		{
			mr.align(ALIGN_DWORD);
			//DECLDATA(ULONG, cui);
			mr.declField("ExceptionHandler", mr.type(TYPEID_DWORD), ATTR_RVA);
		}
	}
	else
	{
		DECLDATAEX(RUNTIME_FUNCTION, cui);
		mr.declField("ChainedUnwindInfo", mr.type("RUNTIME_FUNCTION"));
	}
#endif
}
END_DYNAMIC_TYPE(UNWIND_INFO);



/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(COFF_SymbolEntry)
{
	DECLDATAEX(IMAGE_SYMBOL_ENTRY, u);

	if (u.prime.Zeroes != 0)
		mr.declField("Name", mr.arrayOf(mr.type(TYPEID_CHAR), 8));
	else
	{
		mr.declField("Zeroes", mr.type(TYPEID_DWORD));
		mr.declField("Offset", mr.type(TYPEID_DWORD), (AttrIdEnum)ATTR_COFF_STRING_TABLE_REF);
	}

	mr.declField("Value", mr.type(TYPEID_DWORD), (AttrIdEnum)ATTR_COFF_SYMBOL_VALUE_REF);
	if (u.SectionNumber < 1)
		mr.declField("Section", mr.enumOf(mr.type("IMAGE_SYMBOL_SECTION"), TYPEID_SHORT));
	else
		mr.declField("SectionNumber", mr.type(TYPEID_SHORT));
	if (u.TypeHi != 0)
	{
		mr.declField("TypeLo", mr.enumOf(mr.type("IMAGE_SYMBOL_TYPE"), TYPEID_BYTE));
		mr.declField("TypeHi", mr.enumOf(mr.type("IMAGE_SYMBOL_TYPE_HI"), TYPEID_BYTE));
	}
	else
		mr.declField("Type", mr.enumOf(mr.type("IMAGE_SYMBOL_TYPE"), TYPEID_USHORT));
	mr.declField("StorageClass", mr.enumOf(mr.type("IMAGE_SYMBOL_CLASS"), TYPEID_BYTE));
	mr.declField("NumberOfAuxSymbols", mr.type(TYPEID_BYTE));
}
END_DYNAMIC_TYPE(COFF_SymbolEntry);


/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(WIN_CERTIFICATE)
{
	DECLDATA(DWORD, dwLength);
	mr.declField("Length", mr.type(TYPEID_DWORD));
	mr.declField("Revision", mr.type(TYPEID_WORD));
	mr.declField("CertificateType", mr.enumOf(mr.type("WIN_CERTIFICATE_TYPE"), TYPEID_WORD));
	//BYTE  bCertificate[ANYSIZE_ARRAY];
	mr.declField("Certificate", mr.arrayOf(mr.type(TYPEID_BYTE), dwLength - DWORD(8)));// , ATTR_COLLAPSED);
}
END_DYNAMIC_TYPE(WIN_CERTIFICATE);


/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(_TypeDescriptor)
{
	mr.declField("pVFTable", mr.ptrOf(mr.type(TYPEID_VOID)));//const
	mr.declField("spare", mr.ptrOf(mr.type(TYPEID_VOID)));
	mr.declField("name", toAscii(mr));
}
END_DYNAMIC_TYPE(_TypeDescriptor);

/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(__vmi_class_type_info)
{
	mr.declField("vptr", mr.ptrOf(mr.type(TYPEID_VOID)));
	mr.declField("type_name", mr.ptrOf(mr.type(TYPEID_VOID)));
	//mr.declField("flags", mr.type(TYPEID_INT));
	mr.declBField("non_diamond_repeated", /*mr.arrayOf(*/mr.type(TYPEID_UINT)/*, 1)*/);//0x1		//?virtual_base
	mr.declBField("diamond_shaped", /*mr.arrayOf(*/mr.type(TYPEID_UINT)/*, 1)*/);//0x2		//?public_base
	mr.declField("base_count", mr.type(TYPEID_UINT));
	DECLDATA2(unsigned int, base_count, -(int)sizeof(unsigned int));
	if (base_count > 0)
		mr.declField("base_info", mr.arrayOf(mr.type("__base_class_type_info"), base_count));
}
END_DYNAMIC_TYPE(__vmi_class_type_info);


/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(CV_INFO_PDB20)
{
	mr.declField("CvHeader", mr.type("CV_HEADER"));// NB10
	mr.declField("Signature", mr.type(TYPEID_DWORD));
	mr.declField("Age", mr.type(TYPEID_DWORD));
	mr.declField("PdbFileName", toAscii(mr));
}
END_DYNAMIC_TYPE(CV_INFO_PDB20);

/////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(CV_INFO_PDB70)
{
	mr.declField("CvSignature", mr.arrayOf(mr.type(TYPEID_CHAR), 4));// RSDS
	mr.declField("GuidSignature", mr.arrayOf(mr.type(TYPEID_UINT8), 16));
	mr.declField("Age", mr.type(TYPEID_DWORD));
	mr.declField("PdbFileName", toAscii(mr));
}
END_DYNAMIC_TYPE(CV_INFO_PDB70);

static char isMFMC(char q)
{
	switch (q)
	{
	case 'A': // private
	case 'B': // private:far
	case 'C': // private:static
	case 'D': // private:static far
	case 'E': // private:virtual
	case 'F': // private:virtual far
	case 'I': // protected
	case 'J': // protected:far"
	case 'K': // protected:static
	case 'L': // protected:static far
	case 'M': // protected:virtual
	case 'N': // protected:virtual far
	case 'Q': // public
	case 'R': // public:far
	case 'S': // public:static
	case 'T': // public:static far
	case 'U': // public:virtual
	case 'V': // public:virtual far
		return q;
	}
	return 0;
}

static char isMFMC_static(char q)
{
	switch (q)
	{
	case 'C': // private:static
	case 'D': // private:static far
	case 'K': // protected:static
	case 'L': // protected:static far
	case 'S': // public:static
	case 'T': // public:static far
		return q;
	}
	return 0;
}

static char isCV(char q)
{
	switch (q)
	{
	case 'A': // default
	case 'B': // const
	case 'C': // volatile
	case 'D': // const volatile
		return q;
	}
	return 0;
}

static char isCC(char q)
{
	switch (q)
	{

	case 'A': // __cdecl
	case 'C': // __pascal (__fortran)
	case 'E': // __thiscall
	case 'G': // __stdcall
	case 'I': // __fastcall
	//case 'E':// __regcall
	case 'Q': // __vectorcall
	//case 'A':// interrupt
		return q;
	}
	return 0;
}

static char isONC_(char q)
{
	switch (q)
	{
	case 'U':
	case 'V':
	case '0':
	case '1':
	case '3':
	case '2':
	case '4':
	case '5':
	case '6':
	case '7':
		return q;
	}
	return 0;
}

static char isONC(char q)
{
	switch (q)
	{
	case '0':
	case '1':
	case 'A':
	case 'R':
	case 'C':
	case 'E':
	case 'F':
	case '2':
	case '3':
	case 'D':
	case 'I':
	case 'H':
	case 'G':
	case '7':
	case 'S':
	case 'J':
		//case 'D':
	case 'K':
	case 'L':
		//case 'H':
		//case 'G':
	case '6':
	case '5':
	case 'M':
	case 'O':
	case 'N':
	case 'P':
	case '8':
	case '9':
		//case 'I':
	case 'U':
	case 'T':
	case 'V':
	case 'W':
	case '4':
	case 'X':
	case 'Y':
	case 'Z':
	case 'Q':
	case 'B':
		return q;
	}
	return 0;
}

static char isTC(char q)
{
	switch (q)
	{
	case '_':
	case 'A': // reference
	case 'D': // char
	case 'C': // signed char
	case 'E': // unsigned char
	case 'F': // short int
	case 'G': // unsigned short int
	case 'H': // int
	case 'I': // unsigned int
	case 'J': // long int
	case 'K': // unsigned long int
	//case 'G': // wchar_t
	case 'M': // float
	case 'N': // double
	case 'O': // long double
	case 'P': // pointer(*)<stirage class><base type>
	case 'Q': // const pointer(*)<stirage class><base type>
	case 'T': // union<name>
	case 'U': // struct<name>
	case 'V': // class<name>
	case 'W': // enum <N><name>
	case 'X': // void
		return q;
	}
	return 0;
}

static char isTC_(char q)
{
	switch (q)
	{
	case 'N': // bool
	case 'J': // long long(__int64)
	case 'K': // unsigned long long(unsigned __int64)
	case 'T': // long double
	case 'W': // wchar_t
	case 'Z': // long double
		return q;
	}
	return 0;
}

static char isSCC(char q)//Storage class codes
{
	switch (q)
	{
	case 'A': // near (default)
	case 'B': // const
	case 'C': // volatile
	case 'D': // const volatile
	case 'E': // far
	case 'F': // const far
	case 'G': // volatile far
	case 'H': // const volatile far
	case 'I': // huge
	//case 'F': //__unaligned F
	//case 'I': //__restrict I
		return q;
	}
	return 0;
}

static char isAC(char q)//Member object access code
{
	switch (q)
	{
	case '0': // private
	case '1': // protected
	case '2': // public
		return q;
	}
	return 0;
}

BEGIN_DYNAMIC_TYPE(MANGLED_SCHEME_MSVC)
{
	DECLDATA(char, q1);
	mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
	if (q1 != '?')
		return;

	int iName(0);

	// check for operators
	DECLDATA(char, z1);
	if (z1 == '?')
	{
		mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
		DECLDATA(char, z2);
		if (z2 == '_')
		{
			mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
			mr.declField("special_name", mr.enumOf(mr.type("MSVC_MANGLE_ONC_"), TYPEID_BYTE));
		}
		else
			mr.declField("special_name", mr.enumOf(mr.type("MSVC_MANGLE_ONC"), TYPEID_BYTE));
		iName++;//a scope follows
	}

	for (;; iName++)
	{
		mr.declField(iName == 0 ? "name" : "scope", toAsciiEx(mr, false, '@', 1024));
		DECLDATA(char, q2);
		if (q2 == '@')
		{
			mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
			break;
		}
	}

	//Member function modifier codes

	DECLDATA(char, q3);
	if (!isMFMC(q3))
	{
		if (isAC(q3) || q3 == '3')
		{
			if (q3 == '3')//global objects
				mr.declField("global", mr.type(TYPEID_CHAR));
			else
				mr.declField("access", mr.enumOf(mr.type("MSVC_MANGLE_AC"), TYPEID_CHAR));
			DECLDATA(char, q4);
			char a(isTC(q4));
			if (!a)
				return;
			if (mr.NewScope(mr.declField("type")))
			{
				SAFE_SCOPE_HERE(mr);
				if (!declTypeRef(mr, a))
					return;
			}
			DECLDATA(char, q5);
			if (!isCV(q5))
				return;
			mr.declField("const_vol", mr.enumOf(mr.type("MSVC_MANGLE_CV"), TYPEID_CHAR));
		}
		else if (q3 == '6')//virtual table?
		{
			//mr.declField("vtable", mr.type(TYPEID_CHAR));
			mr.declField("vtable", toAsciiEx(mr, false, '@', 1024));
		}
		return;
	}
	mr.declField("storage_call_type", mr.enumOf(mr.type("MSVC_MANGLE_MFMC"), TYPEID_CHAR));

	if (!isMFMC_static(q3))
	{
		// Member function access codes (storage for `this' target)
		DECLDATA(char, q4);
		if (!isCV(q4))
			return;
		mr.declField("const_vol", mr.enumOf(mr.type("MSVC_MANGLE_CV"), TYPEID_CHAR));
	}

	//Function calling convention codes

	DECLDATA(char, q5);
	if (!isCC(q5))
		return;

	mr.declField("calling_convention", mr.enumOf(mr.type("MSVC_MANGLE_CC"), TYPEID_CHAR));

	// Storage class codes for return

	DECLDATA(char, q6);
	if (q6 == '@')//return type of constructors/destructors
	{
		mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
	}
	else if (mr.NewScope(mr.declField("retval")))
	{
		SAFE_SCOPE_HERE(mr);

		if (q6 == '?')
		{
			mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
			DECLDATA(char, q7);
			if (!isCV(q7))
				return;
			mr.declField("const_vol", mr.enumOf(mr.type("MSVC_MANGLE_CV"), TYPEID_CHAR));
		}

		//return type
		DECLDATA(char, q7);
		char a(isTC(q7));
		if (!a)
			return;
		if (!declTypeRef(mr, a))
			return;
	}

	//parameters
	for (;;)
	{
		DECLDATA(char, a1);
		if (a1 == '@')//term
		{
			mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
			break;
		}
		char a(isTC(a1));
		if (!a)
		{
			//if (a1 != 'Q')//const
			{
				if (isdigit(a1))//abbreviations for repeated parameter types?
				{
					mr.declField("repeated", mr.type(TYPEID_CHAR), (AttrIdEnum)24);//? ATTR_MANGLED_NAME_REF);
					continue;
				}
				return;
			}
		}
		if (mr.NewScope(mr.declField("arg")))
		{
			SAFE_SCOPE_HERE(mr);
			/*if (a1 == 'Q')//const
			{
				mr.declField("const", mr.type(TYPEID_CHAR));
				DECLDATA(char, a2);
				a = a2;
			}*/
			if (!declTypeRef(mr, a))
				return;
			if (a == 'X' || a == 'Z')//void or ...  - no term
				break;
		}
	}

	DECLDATA(char, termZ);
	if (termZ != 'Z')
		return;
	mr.declField("term", mr.type(TYPEID_CHAR));
}
bool declTypeRef(I_Module& mr, char a)
{
	if (a == '_')
	{
		mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
		DECLDATA(char, a2);
		if (!isTC_(a2))
			return false;
		mr.declField("type", mr.enumOf(mr.type("MSVC_MANGLE_TC_"), TYPEID_CHAR));
	}
	else
	{
		mr.declField("type", mr.enumOf(mr.type("MSVC_MANGLE_TC"), TYPEID_CHAR));

		if (a == 'P' || a == 'Q' || a == 'A')//pointer | const pointer | reference
		{
			DECLDATA(char, a2);
			if (!isSCC(a2))
				return false;
			mr.declField("storage_class", mr.enumOf(mr.type("MSVC_MANGLE_SCC"), TYPEID_CHAR));
			DECLDATA(char, a3);
			char b(isTC(a3));
			if (!b)
				return false;
			if (b == '_')
				return false;
			mr.declField("ptr_base_type", mr.enumOf(mr.type("MSVC_MANGLE_TC"), TYPEID_CHAR));
			if (!declTypeRef2(mr, b))
				return false;
		}
		else if (!declTypeRef2(mr, a))
			return false;
	}
	return true;
}
bool declTypeRef2(I_Module& mr, char b)
{
	if (b == 'W')//enum
	{
		DECLDATA(char, a4);
		if (a4 != '4')
			return false;
		mr.declField(nullptr, mr.type(TYPEID_CHAR));//mr.skip(1);
	}
	else if (b != 'T' && b != 'U' && b != 'V')
		return true;
	return declNameScoped(mr);
}
bool declNameScoped(I_Module& mr)
{
	for (int i(0);; i++)
	{
		DECLDATA(char, a5);
		if (isdigit(a5))//a ref to name does not require @ terminator
			mr.declField("ref", mr.type(TYPEID_CHAR), (AttrIdEnum)24);//? ATTR_MANGLED_NAME_REF);
		else if (a5 == '?')//templated name?
		{
			// Template functions and template classes are coded by replacing <function name> or <class name> by ?$ <name> @ [ <template parameters>],
			// where <name> is the name of the templated function or class.
			DECLDATA2(char, a6, 1);
			if (a6 != '$')
				return false;
			if (mr.NewScope(mr.declField("templ_name")))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(i == 0 ? "name" : "scope", toAsciiEx(mr, false, '@', 1024));
				//check for parameters
				for (int j(0);; j++)
				{
					DECLDATA(char, a1);
					if (a1 == '@')
					{
						mr.declField("term", mr.type(TYPEID_CHAR));
						break;
					}

					char a(isTC(a1));
					if (!a)
					{
						if (isdigit(a1))//abbreviations for repeated parameter types?
						{
							mr.declField("repeated", mr.type(TYPEID_CHAR), (AttrIdEnum)24);//? ATTR_MANGLED_NAME_REF);
							continue;
						}
						if (a1 == '$')
						{
							//if a template parameter is a constant => <template parameter> ::= $0 <integer>, where <integer> is:
							// range for N | coding
							//------------------------------------------
							//1 <= N <= 10 | (N - 1) as a decimal number
							// N > 10      | code N as a hexadecimal number without leading zeroes, replace the hexadecimal digits 0 - F by the letters A - P, end with a @
							// N = 0       | A@
							// N < 0       | ? followed by ( - N) coded as above
							DECLDATA2(char, a2, 1);
							if (a2 == '0')
							{
								DECLDATA2(char, a3, 2);
								if (isdigit(a3))
									mr.declField("number", mr.arrayOf(mr.type(TYPEID_CHAR), 3));
								else
									mr.declField("number", toAsciiEx(mr, false, '@', 1024));
								continue;
							}
						}
						return false;
					}
					if (mr.NewScope(mr.declField("arg")))
					{
						SAFE_SCOPE_HERE(mr);
						if (!declTypeRef(mr, a))
							return false;
					}
				}
			}
		}
		else
			mr.declField(i == 0 ? "name" : "scope", toAsciiEx(mr, false, '@', 1024));

		DECLDATA(char, q2);
		if (q2 == '@')
		{
			mr.declField("term_name", mr.type(TYPEID_CHAR));
			break;
		}
	}
	return true;
}
END_DYNAMIC_TYPE(MANGLED_SCHEME_MSVC);





/*struct { const char* a; char b[3]; } call_type[6] =
{ { "default",     'A',       'I',         'Q' },
  { "far",         'B',       'J',         'R' },
  { "static",      'C',       'K',         'S' },
  { "static far",  'D',       'L',         'T' },
  { "virtual",     'E',       'M',         'U' },
  { "virtual far", 'F',       'N',         'V' }
};*/



void PE_declareDynamicTypes(I_ModuleEx &mr)
{
	mr.DeclareContextDependentType(_PFX("IMAGE_RESOURCE_DIRECTORY_ROOT"));
	mr.DeclareContextDependentType(_PFX("IMAGE_RESOURCE_DIRECTORY_ENTRY"));
	mr.DeclareContextDependentType(_PFX("IMAGE_IMPORT_BY_NAME"));
	mr.DeclareContextDependentType(_PFX("DLGTEMPLATE"));
	mr.DeclareContextDependentType(_PFX("DLGITEMTEMPLATE"));
	mr.DeclareContextDependentType(_PFX("DLGITEMTEMPLATEEX"));
	mr.DeclareContextDependentType(_PFX("NormalMenuItem"));
	mr.DeclareContextDependentType(_PFX("PopupMenuItem"));
	mr.DeclareContextDependentType(_PFX("VersionString"));
	mr.DeclareContextDependentType(_PFX("RES_ICON"));
	mr.DeclareContextDependentType(_PFX("RES_BITMAP"));
	mr.DeclareContextDependentType(_PFX("VS_VERSIONINFO"));
	mr.DeclareContextDependentType(_PFX("RES_DIALOG"));
	mr.DeclareContextDependentType(_PFX("RES_MENU"));
	mr.DeclareContextDependentType(_PFX("RES_STRINGS"));
	mr.DeclareContextDependentType(_PFX("UNWIND_INFO"));
	mr.DeclareContextDependentType(_PFX("COFF_SymbolEntry"));
	mr.DeclareContextDependentType(_PFX("WIN_CERTIFICATE"));
	mr.DeclareContextDependentType(_PFX("CV_INFO_PDB70"));
	mr.DeclareContextDependentType(_PFX("CV_INFO_PDB20"));
	mr.DeclareContextDependentType(_PFX("MANGLED_SCHEME_MSVC"));
}
