#include <math.h>
#include "shared.h"
#include "shared/misc.h"
#include "format.misc.h"

/////////////////////////////////////////////////////

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWORD;
typedef long			LONG;

struct ICONDIRENTRY
{
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	DWORD       dwImageOffset;   // Where in the file is this image?
};

struct ICONDIR
{
	WORD  idReserved;   // Reserved (must be 0)
	WORD  idType;       // Resource Type (1 for icons)
	WORD    idCount;      // How many images?
	ICONDIRENTRY	idEntries[1]; // An entry for each image (idCount of 'em)
};

class T_ICO
{
	I_SuperModule &mr;
	HTYPE	mpico;
public:
	T_ICO(I_SuperModule &rMain) : mr(rMain){}

	void createz(int dataSize)
	{
		mpico = mr.NewScope(mr.declField());
		{
			//s->set Name(name());

			mr.installNamespace();
			mr.installTypesMgr();

			createStructures();

			DECLDATAEX(ICONDIR, a);

			mr.declField("Reserved", type(TYPEID_WORD));
			mr.declField("Type", type(TYPEID_WORD));
			mr.declField("Count", type(TYPEID_WORD));

			DECLDATAPTREX(ICONDIRENTRY, pidesc);

			mr.declField("Entries", arrayOf(type("ICONDIRENTRY"), a.idCount));
			//HFIELD fEntries(mr.declField("Entries", arrayOf(type("ICONDIRENTRY"), a.idCount)));
			/*for (int i(0); i < a.idCount; i++)
			{
				//ADDR addr = mr.FieldOffset(fEntries) + i * (sizeof(ICONDIRENTRY));
				//ICONDIRENTRY &idesc = *(ICONDIRENTRY *)&pBuf[addr];
				ICONDIRENTRY &idesc(pidesc[i]);

				PSTRUC pbmp(mr.NewScope(mr.declField()));
				{
					mr.StrucSetSize(pbmp, idesc.dwBytesInRes);

					HFIELD f2(mr.insertField(mpico, idesc.dwImageOffset, "IconImage", pbmp));

					HFIELD f3(mr.declField("InfoHeader", type("BITMAPINFOHEADER")));
					if (idesc.bColorCount > 0)
					{
						HFIELD f4(mr.declField("icColors", arrayOf(type("RGBQUAD"), idesc.bColorCount)));
					}
					mr.Leave();
				}
			}*/
			mr.Leave();
		}
	}

protected:

	HTYPE type(OpType_t i){ return mr.type(i); }
	HTYPE type(HNAME n){ return mr.type(n); }
	HTYPE	arrayOf(HTYPE t, unsigned n){ return mr.arrayOf(t, n); }

	void createStructures()
	{
		if (mr.NewScope("ICONDIRENTRY"))
		{
			mr.declField("Width", type(TYPEID_BYTE));
			mr.declField("Height", type(TYPEID_BYTE));
			mr.declField("ColorCount", type(TYPEID_BYTE));
			mr.declField("Reserved", type(TYPEID_BYTE));
			mr.declField("Planes", type(TYPEID_WORD));
			mr.declField("BitCount", type(TYPEID_WORD));
			mr.declField("SizeInBytes", type(TYPEID_DWORD));
			mr.declField("FileOffset", type(TYPEID_DWORD));
			mr.Leave();
		}

		if (mr.NewScope("BITMAPINFOHEADER"))
		{
			//mr.addType(mpico, s, "BITMAPINFOHEADER");
			mr.declField("Size", type(TYPEID_DWORD));
			mr.declField("Width", type(TYPEID_DWORD));
			mr.declField("Height", type(TYPEID_DWORD));
			mr.declField("Planes", type(TYPEID_WORD));
			mr.declField("BitCount", type(TYPEID_WORD));
			mr.declField("Compression", type(TYPEID_DWORD));
			mr.declField("ImageSize", type(TYPEID_DWORD));
			mr.declField("XpixelsPerM", type(TYPEID_DWORD));
			mr.declField("YpixelsPerM", type(TYPEID_DWORD));
			mr.declField("ColorsUsed", type(TYPEID_DWORD));
			mr.declField("ColorsImportant", type(TYPEID_DWORD));
			mr.Leave();
		}

		if (mr.NewScope("RGBQUAD"))
		{
			//mr.addType(mpico, s, "RGBQUAD");
			mr.declField("rgbBlue", type(TYPEID_BYTE));
			mr.declField("rgbGreen", type(TYPEID_BYTE));
			mr.declField("rgbRed", type(TYPEID_BYTE));
			mr.declField("rgbReserved", type(TYPEID_BYTE));
			mr.Leave();
		}

		if (mr.NewScope("BITMAPINFOHEADER"))
		{
			mr.declField("biSize", type(TYPEID_DWORD));
			mr.declField("biWidth", type(TYPEID_LONG));
			mr.declField("biHeight", type(TYPEID_LONG));
			mr.declField("biPlanes", type(TYPEID_WORD));
			mr.declField("biBitCount", type(TYPEID_WORD));
			mr.declField("biCompression", type(TYPEID_DWORD));
			mr.declField("biSizeImage", type(TYPEID_DWORD));
			mr.declField("biXPelsPerMeter", type(TYPEID_LONG));
			mr.declField("biYPelsPerMeter", type(TYPEID_LONG));
			mr.declField("biClrUsed", type(TYPEID_DWORD));
			mr.declField("biClrImportant", type(TYPEID_DWORD));
			mr.Leave();
		}
	}

};



class T_BMP
{
	I_SuperModule &mr;
	HTYPE	mp;
protected:

	HTYPE type(OpType_t i){ return mr.type(i); }
	HTYPE type(HNAME n){ return mr.type(n); }
	HTYPE	arrayOf(HTYPE t, unsigned n){ return mr.arrayOf(t, n); }

	void createStructures()
	{
		if (mr.NewScope("BITMAPFILEHEADER"))
		{
			mr.declField("Type", arrayOf(type(TYPEID_CHAR), 2));
			mr.declField("Size", type(TYPEID_DWORD));
			mr.declField("Reserved1", type(TYPEID_WORD));
			mr.declField("Reserved2", type(TYPEID_WORD));
			mr.declField("OffBits", type(TYPEID_DWORD));
			mr.Leave();
		}

		if (mr.NewScope("BITMAPINFOHEADER"))
		{
			mr.declField("Version", type(TYPEID_DWORD));//Size
			mr.declField("Width", type(TYPEID_LONG));
			mr.declField("Height", type(TYPEID_LONG));
			mr.declField("Planes", type(TYPEID_WORD));
			mr.declField("BitsPerPixel", type(TYPEID_WORD));
			mr.declField("Compression", type(TYPEID_DWORD));
			mr.declField("SizeOfBitmap", type(TYPEID_DWORD));
			mr.declField("HorzResolution", type(TYPEID_LONG));
			mr.declField("VertResolution", type(TYPEID_LONG));
			mr.declField("ColorsUsed", type(TYPEID_DWORD));
			mr.declField("ColorsImportant", type(TYPEID_DWORD));
			mr.Leave();
		}

		if (mr.NewScope("RGBQUAD"))
		{
			mr.declField("Blue", type(TYPEID_BYTE));
			mr.declField("Green", type(TYPEID_BYTE));
			mr.declField("Red", type(TYPEID_BYTE));
			mr.declField("Reserved", type(TYPEID_BYTE));
			mr.Leave();
		}

		if (mr.NewScope("RGBTRIPLE"))
		{
			mr.declField("Blue", type(TYPEID_BYTE));
			mr.declField("Green", type(TYPEID_BYTE));
			mr.declField("Red", type(TYPEID_BYTE));
			mr.Leave();
		}
	}

public:
	T_BMP(I_SuperModule &rMain) : mr(rMain){}
	void createz(int dataSize)
	{
		if (mp = mr.NewScope(mr.declField()))
		{
			mr.setSize(dataSize);
			//s->set Name(name());

			mr.installNamespace();
			mr.installTypesMgr();

			createStructures();

			struct BITMAPINFOHEADER
			{
				DWORD      biSize;
				LONG       biWidth;
				LONG       biHeight;
				WORD       biPlanes;
				WORD       biBitCount;
				DWORD      biCompression;
				DWORD      biSizeImage;
				LONG       biXPelsPerMeter;
				LONG       biYPelsPerMeter;
				DWORD      biClrUsed;
				DWORD      biClrImportant;
			};
			
			DECLDATAEX(BITMAPINFOHEADER, bmih);
			
			mr.declField("FileHeader", type("BITMAPFILEHEADER"));

			mr.declField("Header", type("BITMAPINFOHEADER"));

#if(0)
			// Define the color table
			if ((bmih.biBitCount != 24) && (bmih.biBitCount != 32))
			{
				if (bmih.biClrUsed > 0)
					mr.declField("Colors", arrayOf(type("RGBQUAD"), bmih.biClrUsed));
				else
					mr.declField("Colors", arrayOf(type("RGBQUAD"), 1 << bmih.biBitCount));
			}

			if (bmih.biCompression > 0)
			{
				// Bytes are compressed
				if (bmih.biSizeImage > 0)
					mr.declField("rleData", arrayOf(type(TYPEID_BYTE), bmih.biSizeImage));
				else
				{assert(0);}//UBYTE rleData[bmfh.bfSize - FTell()];
			}
			else
			{
				// Calculate bytes per line and padding required
				int bytesPerLine = (int)ceil(bmih.biWidth * bmih.biBitCount / 8.0);
				int padding = 4 - (bytesPerLine % 4);
				if (padding == 4)
					padding = 0;

				// Define each line of the image
				int lines((bmih.biHeight < 0) ? -bmih.biHeight : bmih.biHeight);
				for (int i(0); i < lines; i++)
				{
					// Define color data
					if (bmih.biBitCount < 8)
						mr.declField("imageData", arrayOf(type(TYPEID_BYTE), bytesPerLine));
					else if (bmih.biBitCount == 8)
						mr.declField("colorIndex", arrayOf(type(TYPEID_BYTE), bmih.biWidth));
					else if (bmih.biBitCount == 24)
						mr.declField("colors", arrayOf(type("RGBTRIPLE"), bmih.biWidth));
					else if (bmih.biBitCount == 32)
						mr.declField("colors", arrayOf(type("RGBQUAD"), bmih.biWidth));

					// Pad if necessary        
					if (padding != 0)
						mr.skip(padding);
				}
			}
#endif

			mr.Leave();
		}
	}
};


class CDynamicType_ICO : public I_FormatterType
{
protected:
	virtual const char *name() const { return "RC_ICO"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_ICO ico(r);
		ico.createz(nSize);
	}
};


class CDynamicType_BMP : public I_FormatterType
{
public:
	CDynamicType_BMP(){}
	virtual const char *name() const { return "RC_BMP"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_BMP bmp(r);
		bmp.createz(nSize);
	}
};









///////////////////////////////////////////////////////////////////

class T_SILOS_SIM
{
	I_SuperModule& mr;
	HTYPE	mptop;

public:
	T_SILOS_SIM(I_SuperModule& rMain)
		: mr(rMain)
	{
	}

	void preformat(int dataSize)
	{
		mr.setEndianness(false);//MSB

		mr.installFrontend(_PFX("FE_SILOS"));

		if ((mptop = mr.NewScope(mr.declField())) != HTYPE())
			//		if ((mptop = mr.NewSegment(dataSize, nullptr, I_SuperModule::ISEG_MSB)) != nullptr)
		{
			SAFE_SCOPE_HERE(mr);
			HPATCHMAP hTrz(mr.newPatchMap());

			mr.installNamespace();
			mr.installTypesMgr();

			createStructures();
			mr.DeclareContextDependentType(_PFX("SilosString"));
			mr.DeclareContextDependentType(_PFX("SilosState"));
			mr.DeclareContextDependentType(_PFX("SilosInt"));
			mr.DeclareContextDependentType(_PFX("SilosDouble"));
			mr.DeclareContextDependentType(_PFX("SilosDLong"));
			mr.DeclareContextDependentType(_PFX("SilosTimeBlock"));

			//DECLDATAPTR(unsigned, u);
			mr.declField(nullptr, type("ControlBlock"));
			//mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//END
#pragma pack(push,1)
			struct h_t
			{
				unsigned char id;
				unsigned intIndex;
				unsigned assignIndex;
				unsigned dblIndex;
				unsigned dintIndex;
				unsigned emptyIndex;
			};
#pragma pack(pop)
			DECLDATAEX(h_t, h);
			mr.declField("Header", type("InitBlock"));
			mr.declField("Version", mr.type(_PFX("SilosString")));
			mr.declField("Date", mr.type(_PFX("SilosString")));
			mr.declField("TimeScale", mr.type(_PFX("SilosString")));
			if (mr.NewScope(mr.declField("NameIndicies", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				for (;;)
				{
					DECLDATA(unsigned char, e);
					if (e != 0xA5)
						break;
					mr.declField(nullptr, type("IndexBlock"));
				}
			}
			mr.declField(nullptr, type("ChkptBlock"));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ACTUAL_VALUES_BLOCK
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//END_INIT_ID

			//mr.declField(nullptr, type("ControlBlock"));

			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));
			mr.declField(nullptr, type("DumpBlock"));
			mr.declField(nullptr, type("ChkptBlock"));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ACTUAL_VALUES_BLOCK
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ITER_BLOCK

			if (mr.NewScope(mr.declField("IntIndicies", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				unsigned l(swap_endian(h.intIndex));
				for (size_t i(0); i < l; i++)
				{
					OFF_t r(mr.cpr());
					//CHECK(mr.cpr() >= 0x6ab)
					//STOP
						//			DECLDATA(unsigned char, e);
							//		if (e & 0x80)//BLOCK
								//		break;
					mr.declField(nullptr, mr.type(_PFX("SilosState")));
				}
			}
			if (mr.NewScope(mr.declField("DblIndicies", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				unsigned l(4);// swap_endian(h.dblIndex));
				for (size_t i(0); i < l; i++)
				{
					mr.declField(nullptr, mr.type(_PFX("SilosDouble")));
				}
			}
			
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CHANGED_BLOCK
			if (mr.NewScope(mr.declField("LastChange", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				for (;;)
				{
					DECLDATA(unsigned char, e);
					if (e & 0x80)
						break;
					mr.declField(nullptr, mr.type(_PFX("SilosDLong")));
				}
			}
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//END_CHANGED_BLOCK

			mr.declField(nullptr, type("DumpFactsBlock"));
			mr.declField(nullptr, type("ChkptBlock"));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ACTUAL_VALUES_BLOCK
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//END_DUMP_BLOCK

			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ACTUAL_VALUES_BLOCK
//			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CR_HALT0_BLOCK
			mr.declField(nullptr, type("HaltBlock"));
			

			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//RESUME_BLOCK

			for (int j = 0; j < 100; j++)
			{
				DECLDATA(unsigned char, u);
				if (u < 0xA0 || u > 0xA2)
					break;

				if (mr.NewScope(mr.declField(nullptr, ATTR_NEWLINE)))//ATTR_COLLAPSED))
				{
					SAFE_SCOPE_HERE(mr);

					mr.declField(nullptr, mr.type(_PFX("SilosTimeBlock")));
					mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ITER_BLOCK

					for (int i = 0; i < 4; i++)
					{
						//mr.declField(nullptr, mr.type(_PFX("SilosInt")));
						mr.declField(nullptr, mr.type(_PFX("SilosDouble")));
					}
				}
			}

			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ITER_BLOCK

			if (mr.NewScope(mr.declField("IntIndicies2", ATTR_NEWLINE)))
			{
				SAFE_SCOPE_HERE(mr);
				unsigned l(swap_endian(h.intIndex));
				for (size_t i(0); i < l; i++)
				{
					OFF_t r(mr.cpr());
					//CHECK(mr.cpr() >= 0x6ab)
					//STOP
						//			DECLDATA(unsigned char, e);
							//		if (e & 0x80)//BLOCK
								//		break;
					mr.declField(nullptr, mr.type(_PFX("SilosState")));
				}
			}
		}
	}

protected:

	HTYPE type(OpType_t i) { return mr.type(i); }
	HTYPE type(HNAME n) { return mr.type(n); }
	HTYPE	arrayOf(HTYPE t, unsigned n) { return mr.arrayOf(t, n); }

	void createStructures()
	{
		if (mr.NewScope("CR_CNTL", SCOPE_ENUM))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declEField("END", 0x0);
			mr.declEField("FULL_DUMP", 0x1);
			mr.declEField("TIME", 0x2);
			mr.declEField("LAST", 0x3);
			mr.declEField("SIM_RUNNING", 0x4);
			mr.declEField("SIM_NOT_RUNNING", 0x5);
			mr.declEField("SIM_LINIT_DONE", 0x6);
			mr.declEField("SIM_LINIT_NOT_DONE", 0x7);
			mr.declEField("WRITE_SEEK", 1 << 3);
			mr.declEField("RESET", 1 << 4);
			mr.declEField("CLOSE", 1 << 5);
			mr.declEField("REMOVE", 1 << 6);
			mr.declEField("NEXT_TIME_ID", 0x80);
			mr.declEField("STRING_BLOCK", 0x81);
			mr.declEField("ITER_BLOCK", 0x82);
			mr.declEField("ALL_TIMES_ID", 0x83);
			mr.declEField("SOFT_END_OF_DATA_ID", 0x84);
			mr.declEField("END_INIT_BLOCK", 0x9D);
			mr.declEField("END_DUMP_BLOCK", 0x9E);
			mr.declEField("TIME1_BLOCK", 0xA0);
			mr.declEField("TIME2_BLOCK", 0xA1);
			mr.declEField("TIME8_BLOCK", 0xA2);
			mr.declEField("ADVANCE_TIME_BLOCK", 0xA3);
			mr.declEField("NAME_INDEX_BLOCK", 0xA4);
			mr.declEField("INDEX_BLOCK", 0xA5);
			mr.declEField("NOP_BLOCK", 0xA6);
			mr.declEField("BLOCK", 0xA7);
			mr.declEField("ACTUAL_VALUES_BLOCK", 0xA8);
			mr.declEField("DUMP_BLOCK", 0xA9);
			mr.declEField("DUMP_FACTS_BLOCK", 0xAA);
			mr.declEField("HALT0_BLOCK", 0xAB);
			mr.declEField("HALT1_BLOCK", 0xAC);
			mr.declEField("RESUME_BLOCK", 0xAD);
			mr.declEField("CHKPT_BLOCK", 0xAE);
			mr.declEField("INIT_BLOCK", 0xAF);
			mr.declEField("NOCONVERGE_BLOCK", 0xB0);
			mr.declEField("RE_NOCONVERGE_BLOCK", 0xB1);
			mr.declEField("STRING_DONE_BLOCK", 0xB2);
			mr.declEField("LAST_CHANGED_BLOCK", 0xB3);
			mr.declEField("END_LAST_CHANGED_BLOCK", 0xB4);
		}

		if (mr.NewScope("ControlBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//BLOCK
			mr.declField("Flags", type(TYPEID_BYTE));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//TIME
			mr.declField("TimeStart", type(TYPEID_QWORD));
			mr.declField("TimeStop", type(TYPEID_QWORD));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//LAST
			mr.declField("PosLast", type(TYPEID_QWORD), ATTR_FILEPTR);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//WRITE_SEEK
			mr.declField("PosSeek", type(TYPEID_QWORD), ATTR_FILEPTR);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//RESET
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CLOSE
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//REMOVE
			mr.declField("eSimStatus", mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//SIM_RUNNING|SIM_NOT_RUNNING
			mr.declField("eLInitStatus", mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//SIM_LINIT_DONE|SIM_LINIT_NOT_DONE
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//END
		}

		if (mr.NewScope("InitBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//INIT_BLOCK
			mr.declField("IntIndex", type(TYPEID_DWORD));
			mr.declField("AssignIndex", type(TYPEID_DWORD));
			mr.declField("DblIndex", type(TYPEID_DWORD));
			mr.declField("DintIndex", type(TYPEID_DWORD));
			mr.declField("EmptyIndex", type(TYPEID_DWORD));
		}

		if (mr.NewScope("IndexBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//INDEX_BLOCK
			mr.declField("Flag", type(TYPEID_BYTE));
			mr.declField("Index", type(TYPEID_DWORD));
		}

		if (mr.NewScope("ChkptBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CHKPT_BLOCK
			mr.declField("Type", type(TYPEID_BYTE));
			mr.declField("Offset", type(TYPEID_QWORD));
		}

		if (mr.NewScope("DumpBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//DUMP_BLOCK
			mr.declField("FullOffset", type(TYPEID_QWORD));
			mr.declField("LastOffset", type(TYPEID_QWORD));
			mr.declField("Time", type(TYPEID_QWORD));
		}

		if (mr.NewScope("DumpFactsBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//DUMP_FACTS_BLOCK
			mr.declField("_approx", type(TYPEID_DWORD));
			mr.declField("_nLogic", type(TYPEID_DWORD));
			mr.declField("_nInt", type(TYPEID_DWORD));
			mr.declField("_nDbl", type(TYPEID_DWORD));
			mr.declField("_nDInt", type(TYPEID_DWORD));
			mr.declField("_nAsgn", type(TYPEID_DWORD));
		}

		if (mr.NewScope("HaltBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CR_HALT0_BLOCK
			mr.declField("_time", type(TYPEID_QWORD));
			mr.declField("nextBlockOffset", type(TYPEID_QWORD));
			mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CR_HALT1_BLOCK
			mr.declField("_time2", type(TYPEID_QWORD));
			mr.declField("nextBlockOffset2", type(TYPEID_QWORD));
		}
	}

public:
	static int sizeof_SB(const I_DataSourceBase& r, OFF_t o)
	{
		int sz(1);
		DataStream_t p(r, o);
		unsigned char u(p.read<unsigned char>());
		if (!(u & 0x40))
			return 2;
		if (!(u & 0x20))
			return 3;
		return 4;
	}
	static HTYPE type_SB(I_Module& r)
	{
		int sz(sizeof_SB(r, r.cpr()));
		OpType_t t(MAKETYP_UINT(sz));
		HTYPE h(r.type(t));
		if (!h)
			h = r.arrayOf(r.type(OPSZ_BYTE), sz);
		return h;
	}

	static int sizeof_DLong(const I_DataSourceBase& r, OFF_t o)
	{
		DataStream_t p(r, o);
		unsigned char u(p.read<unsigned char>());
		if (u & 0x80)
			return (u & 0x40) ? 9 : 4;
		return (u & 0x40) ? 2 : 1;
	}
	static HTYPE type_DLong(I_Module& r)
	{
		int sz(sizeof_DLong(r, r.cpr()));
		OpType_t t(MAKETYP_UINT(sz));
		HTYPE h(r.type(t));
		if (!h)
			h = r.arrayOf(r.type(OPSZ_BYTE), sz);
		return h;
	}
};

BEGIN_DYNAMIC_TYPE(SilosString)
{
	mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//STRING_VERSION
	mr.declField("Key", mr.type(TYPEID_BYTE));
	DECLDATA(unsigned short, l);
	unsigned lu(swap_endian(l.get()));
	mr.declField("Length", mr.type(TYPEID_WORD));
	mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_CHAR), lu));
	mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//CR_STRING_DONE_BLOCK
}
END_DYNAMIC_TYPE(SilosString);

BEGIN_DYNAMIC_TYPE(SilosState)
{
	mr.declField("DeltaIndex", T_SILOS_SIM::type_SB(mr));
	mr.declField("State", mr.type(TYPEID_BYTE));
}
END_DYNAMIC_TYPE(SilosState);

BEGIN_DYNAMIC_TYPE(SilosInt)
{
	mr.declField("DeltaIndex", T_SILOS_SIM::type_SB(mr));
	DECLDATA(unsigned char, u);
	if (!(u & 0x80))
		mr.declField("Value", mr.type(TYPEID_WORD));
	else
	{
		mr.declBField("Code", mr.type(TYPEID_WORD));
		mr.declField("Value", mr.type(TYPEID_DWORD));
	}
}
END_DYNAMIC_TYPE(SilosInt);

BEGIN_DYNAMIC_TYPE(SilosDouble)
{
	mr.declField("DeltaIndex", T_SILOS_SIM::type_SB(mr));
	mr.declField("Value", mr.type(TYPEID_DOUBLE));
}
END_DYNAMIC_TYPE(SilosDouble);

BEGIN_DYNAMIC_TYPE(SilosDLong)
{
	mr.declField("DeltaIndex", T_SILOS_SIM::type_SB(mr));
	mr.declField("Value", T_SILOS_SIM::type_DLong(mr));
}
END_DYNAMIC_TYPE(SilosDLong);

BEGIN_DYNAMIC_TYPE(SilosTimeBlock)
{
	DECLDATA(unsigned char, u);
	mr.declField("Id", mr.type(TYPEID_BYTE));
	OpType_t t;
	if (u == 0xA0)//CR_TIME1_ID
		t = TYPEID_BYTE;
	else if (u == 0xA1)//CR_TIME2_ID
		t = TYPEID_WORD;
	else// if (u == 0xA2)//CR_TIME8_ID
		t = TYPEID_QWORD;
	mr.declField("Value", mr.type(t));
	mr.declField(nullptr, mr.enumOf(mr.type("CR_CNTL"), TYPEID_BYTE));//ADVANCE_TIME_BLOCK
}
END_DYNAMIC_TYPE(SilosTimeBlock);



class FE_SILOS_t : public I_Front
{
public:
	FE_SILOS_t(const I_DataSourceBase*)
	{
	}
protected:
	virtual void release()
	{
		delete this;
	}
	virtual AKindEnum translateAddress(const I_DataStreamBase& mr, int moduleId, ADDR& addr, AttrIdEnum attr)
	{
		ADDR atAddr(mr.cp());
		switch (attr)
		{
		case ATTR_FILEPTR:
			return AKIND_RAW;
		case ATTR_OFFS:
			return AKIND_VA;
		default:
			break;
		}
		return AKIND_NULL;
	}
};

I_Front* CreateFE_SILOS(const I_DataSourceBase* aRaw)
{
	return new FE_SILOS_t(aRaw);
}

//SILOS - SIM
class CDynamicType_SILOS_SIM : public I_FormatterType
{
protected:
	virtual const char* name() const { return "SILOS_SIM"; }
	virtual void createz(I_SuperModule& r, unsigned long nSize)
	{
		T_SILOS_SIM a(r);
		a.preformat(nSize);
	}
};


void ICO_RegisterFormatters(I_ModuleEx& rMain)
{
	rMain.RegisterFormatterType(_PFX("RC_BMP"));
}

///////////////////////////////////////////////////////////////

DECLARE_FORMATTER(CDynamicType_BMP, RC_BMP);
DECLARE_FORMATTER(CDynamicType_ICO, RC_ICO);
DECLARE_FORMATTER(CDynamicType_SILOS_SIM, SILOS_SIM);




