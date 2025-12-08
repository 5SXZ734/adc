#include <math.h>
#include "shared.h"
#include "shared/misc.h"
#include "format.misc.h"

/////////////////////////////////////////////////////////////////

enum HS_FORMAT_e { HS_TR, HS_SW, HS_AC, HS_FT };

struct FirstBlock
{
	uint32_t u0;//4
	uint32_t SublksNum;
	uint32_t u2;//4
	uint32_t BytesInBlock;
};

struct Block
{
	uint32_t BytesPrevBlock;
	uint32_t u0;//4
	uint32_t SublksNum;
	uint32_t u2;//4
	uint32_t BytesInBlock;
};


class T_TR0
{
	I_SuperModule &mr;
	HTYPE	mptop;
	HS_FORMAT_e	meFormat;

protected:
	bool mbReverseOrder;
	template <typename U> U _X(U u) { return mbReverseOrder ? swap_endian(u) : u; }

public:
	T_TR0(I_SuperModule &rMain, HS_FORMAT_e e)
		: mr(rMain),
		meFormat(e),
		mbReverseOrder(false)
	{
	}

	void preformat(int dataSize)
	{
		DECLDATA(unsigned, id);
		if (id != 4)
		{
			if (swap_endian(id.get()) != 4)
				return;//wrong format?
			mbReverseOrder = true;
			mr.setEndianness(false);//MSB
		}

		if ((mptop = mr.NewScope(mr.declField())) != HTYPE())
		{
			SAFE_SCOPE_HERE(mr);
			HPATCHMAP hTrz(mr.newPatchMap());

			mr.installNamespace();
			mr.installTypesMgr();

			createStructures();

			DECLDATAPTR(unsigned, u);
			mr.declField(nullptr, type("FirstBlock"));//, ATTR_COLLAPSED);
			if (_X(u[3]) > 0)
			{
				DECLDATAPTR(char, p);
				//ofs.write(p, _X(u[3]));
				mr.addPatch(hTrz, p.cpr(), _X(u[3]));
				mr.declField(nullptr, arrayOf(type(TYPEID_CHAR), _X(u[3])), ATTR_COLLAPSED);
				unsigned uPrev(_X(u[3]));

				for (int i(0); !mr.isNull(mr.cp()); i++)//final block may go missing?
				{
					DECLDATA(unsigned, v);
					if (_X(v.get()) != uPrev)
						break;
					if (mr.cp() + 4 >= (unsigned)dataSize)
					{
						mr.declField("FinalBlock", type(TYPEID_DWORD));//, ATTR_COLLAPSED);
						break;
					}
					DECLDATAPTR(unsigned, u);
					mr.declField(nullptr, type("Block"));//, ATTR_COLLAPSED);
					DECLDATAPTR(char, p);
					mr.addPatch(hTrz, p.cpr(), _X(u[4]));
					mr.declField(nullptr, arrayOf(type(TYPEID_CHAR), _X(u[4])), ATTR_COLLAPSED);
					uPrev = _X(u[4]);
				}
			}
			if (meFormat == HS_TR)
				mr.stitchDerivativeModule(hTrz, "trz", _PFX("HSPICE_TRZ"));
			else if (meFormat == HS_SW)
				mr.stitchDerivativeModule(hTrz, "swz", _PFX("HSPICE_SWZ"));
			else if (meFormat == HS_AC)
				mr.stitchDerivativeModule(hTrz, "acz", _PFX("HSPICE_ACZ"));
		}
	}

protected:

	HTYPE type(OpType_t i){ return mr.type(i); }
	HTYPE type(HNAME n){ return mr.type(n); }
	HTYPE	arrayOf(HTYPE t, unsigned n){ return mr.arrayOf(t, n); }

	void createStructures()
	{
		if (mr.NewScope("FirstBlock"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField(nullptr, type(TYPEID_DWORD));//4
			mr.declField("SublksNum", type(TYPEID_DWORD));
			mr.declField(nullptr, type(TYPEID_DWORD));//4
			mr.declField("BytesInBlock", type(TYPEID_DWORD));
		}

		if (mr.NewScope("Block"))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField("BytesPrevBlock", type(TYPEID_DWORD));
			mr.declField(nullptr, type(TYPEID_DWORD));//4
			mr.declField("SublksNum", type(TYPEID_DWORD));
			mr.declField(nullptr, type(TYPEID_DWORD));//4
			mr.declField("BytesInBlock", type(TYPEID_DWORD));
		}
	}

};




/////////////////////////////////////////////////////////////////

class T_TRZ
{
	I_SuperModule &mr;
	HTYPE	mptop;
	HS_FORMAT_e meFormat;
protected:
	bool mbReverseOrder;
	template <typename U> U _X(U u) { return mbReverseOrder ? swap_endian(u) : u; }
public:
	T_TRZ(I_SuperModule &rMain, HS_FORMAT_e e)
		: mr(rMain),
		meFormat(e),
		mbReverseOrder(false)
	{
	}

#define TR0_HEADER_SIZE 0xB8
#define	TR0_HEADER_MAX_SIZE 25

	struct TR0_HEADER_t
	{
		enum { magic, title, date, copyright };
		int _header[4];//magic+title+date+copyright
		int version_num;
		//bool bWtf;
		bool bVectorsNum10k;
		bool bProbesNum10k;
		unsigned sweep_info_offset;
		int num_vectors;
		int num_probed;
		int num_swept;
		bool bScaleIsFloat;
		bool bSignalIsFloat;

		TR0_HEADER_t(const char buf[TR0_HEADER_MAX_SIZE])
			: //magic_width(0),
			version_num(0),
			//bWtf(false),
			bVectorsNum10k(false),
			bProbesNum10k(false),
			sweep_info_offset(TR0_HEADER_SIZE),//seems to be always at a fixed offset
			num_vectors(0),
			num_probed(0),
			num_swept(0),
			bScaleIsFloat(false),
			bSignalIsFloat(false)
		{
			_header[magic] = 0;
			_header[title] = 72;
			_header[date] = 32;
			_header[copyright] = 0;

			int d[5];
			memset(&d, 0, sizeof(d));

			int i(0);
			for (const char *p(buf); i < TR0_HEADER_MAX_SIZE && isdigit(*p); p++, i++)
				_header[magic]++;

			int _version_num;
			if (_header[magic] == 25)
			{
				if (sscanf(buf, "%04d%04d%05d%04d%04d%04d", &d[0], &d[1], &d[2], &d[3], &d[4], &_version_num) != 6)
					return;
				bVectorsNum10k = bProbesNum10k = true;
				_header[magic] = 71;
			}
			else if (_header[magic] == 24)
			{
				if (sscanf(buf, "%04d%04d%04d%04d%04d%04d", &d[0], &d[1], &d[2], &d[3], &d[4], &_version_num) != 6)
					return;
				bVectorsNum10k = bProbesNum10k = true;
				_header[date] = 24;
			}
			else if (_header[magic] == 21)
			{
				if (sscanf(buf, "%04d%04d%05d%04d%04d", &d[0], &d[1], &d[2], &d[3], &_version_num) != 5)
					return;
				bVectorsNum10k = true;
				_header[magic] = 75;
			}
			else if (_header[magic] == 20)//9601
			{
				if (sscanf(buf, "%04d%04d%04d%04d%04d", &d[0], &d[1], &d[2], &d[3], &_version_num) != 5)
					return;
				bVectorsNum10k = true;
				//_header[title] = 76;
				_header[date] = 25;
			}
			else
			{
				if (sscanf(buf, "%04d%04d%04d%04d", &d[0], &d[1], &d[2], &_version_num) != 4)
					return;
				//bWtf = true;
			}

			num_vectors = d[0] + d[1] + d[3] * 10000 + d[4] * 10000;
			num_probed = d[1] + d[4] * 10000;
			num_swept = d[2];

			if (_version_num == 9601 || _version_num == 9007)
			{
				bScaleIsFloat = true;
				bSignalIsFloat = true;
			}
			else if (_version_num == 9007)
			{
			}
			else if (_version_num == 2001)
			{
			}
			else if (_version_num == 2013)
			{
				bSignalIsFloat = true;
			}
			else
				return;

			version_num = _version_num;
		}
		int swept_width() const {
			if (_header[magic] == 21 || _header[magic] == 25)
				return 5;
			return 4;
		}
		const int header(size_t i){ return _header[i]; }
	};

	static bool checkDateAt(const char* buf)
	{
		const char* list[7] = { "Fri ", "Mon ", "Sat ", "Sun ", "Thu ", "Tue ", "Wed " };
		size_t len(7);
		size_t start = 0;
		size_t end = len;
		while (start < end)
		{
			size_t middle = (start + end) / 2;
			int comp = strncmp(list[middle], buf, 4);
			if (comp < 0)
				start = middle + 1;
			else if (comp > 0)
				end = middle;
			else
				return 1;
		}
		const char* fmt = "**/**/****";
		for (size_t i(0); fmt[i]; i++)
		{
			if (fmt[i] == '*')
			{
				if (!isdigit(buf[i]))
					return 0;
			}
			else if (fmt[i] != buf[i])
				return 0;
		}
		return 1;
	}

	void preformat(int dataSize)
	{
		//check endianness
		I_DataSourceBase* p0(mr.module(nullptr));//get the primary module
		FirstBlock B1;
		if (p0 && p0->dataAt(0, sizeof(B1), (PDATA)&B1) == sizeof(B1))
		{
			if (B1.u0 != 4)
			{
				if (swap_endian(B1.u0) != 4)
					return;//wrong format?
				mbReverseOrder = true;
				mr.setEndianness(false);//MSB
			}
		}

		if ((mptop = mr.NewScope(mr.declField())) != HTYPE())
		{
			SAFE_SCOPE_HERE(mr);

#if(1)
			mr.setSize(dataSize);
#endif
			mr.installNamespace();
			mr.installTypesMgr();

			createStructures();

			//3 first fields are fixed
			DECLDATA(char[TR0_HEADER_SIZE], p);
			TR0_HEADER_t h(p);

			mr.declField("VectorsNum", arrayOf(type(TYPEID_CHAR), 4));
			mr.declField("ProbesNum", arrayOf(type(TYPEID_CHAR), 4));
			mr.declField("SweptNum", arrayOf(type(TYPEID_CHAR), h.swept_width()));

			if (!h.version_num)
				return;

			//if (h.bWtf)
				//mr.skip(4);

			if (h.bVectorsNum10k)
				mr.declField("VectorsNum10k", arrayOf(type(TYPEID_CHAR), 4));

			if (h.bProbesNum10k)
				mr.declField("ProbesNum10k", arrayOf(type(TYPEID_CHAR), 4));

			mr.declField("VersionNum", arrayOf(type(TYPEID_CHAR), 4));

			if (mr.cp() != h.header(h.magic))
				return;
	
			int title_width(h.header(h.title));//72 by default
			//if (h.header(h.magic) == 20)//9601
			{
				if (checkDateAt(h.header(h.magic) + &p[64]))
					title_width = 64;
				else if (checkDateAt(h.header(h.magic) + &p[68]))
					title_width = 68;
				else if (checkDateAt(h.header(h.magic) + &p[76]))
					title_width = 76;
			}

			mr.declField("Title", arrayOf(type(TYPEID_CHAR), title_width));
			mr.declField("Date", arrayOf(type(TYPEID_CHAR), h.header(h.date)));
			mr.declField("Copyright", arrayOf(type(TYPEID_CHAR), h.sweep_info_offset - (unsigned)mr.cpr()));

			int num_sweeps(0);
			DECLDATA(char[8], g);
			sscanf(g, "%3d", &num_sweeps);
			mr.declField("SweepsInfo", arrayOf(type(TYPEID_CHAR), 8));

			mr.declField(nullptr, arrayOf(type(TYPEID_CHAR), 72));
			
			mr.declField("VectorTypes", arrayOf(arrayOf(type(TYPEID_CHAR), 8), h.num_vectors), ATTR_COLLAPSED);

			static const int HEADER_COLUMN_UNIT = 16;

			if (mr.NewScope(mr.declField("VectorNames", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				for (int i(0); i < h.num_vectors; i++)
				{
					DataStream_t ds(mr, mr.cpr());
					std::stringstream ss;
					ds.fetchString(ss, -1, ' ');
					int n(0);
					DECLDATAPTR(const char, p);
					for (; *p != ' '; ++p)
						n++;
					int sp(0);//trailing spaces
					for (; *p == ' '; ++p)
						sp++;
					n += sp;
					if (n % HEADER_COLUMN_UNIT != 0)
						n = ((n / HEADER_COLUMN_UNIT) + 1) * HEADER_COLUMN_UNIT;
					char buf[32];
					sprintf(buf, "v#%d", i);
					mr.declField(buf, arrayOf(type(TYPEID_CHAR), n));
				}
			}

			int num_swept_variables = 0;

			if (h.num_swept > 0)
			if (mr.NewScope(mr.declField("SweptVectorNames", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);
				for (int i(0); i < h.num_swept; i++, num_swept_variables++)
				{
					int n(1);
					for (DECLDATAPTR(const char, p); *p != ' '; ++p)
						n++;
					if (n % HEADER_COLUMN_UNIT != 0)
						n = ((n / HEADER_COLUMN_UNIT) + 1) * HEADER_COLUMN_UNIT;
					char buf[32];
					sprintf(buf, "sv#%d", i);
					mr.declField(buf, arrayOf(type(TYPEID_CHAR), n));
				}
			}

			{
				const char* HEADER_END_TOKEN = "$&%#    ";

				DataStream_t ds(mr, mr.cpr());
				if (ds.strCmp(HEADER_END_TOKEN, strlen(HEADER_END_TOKEN)) != 0)
					mr.error("HeaderEndToken expected");

				mr.declField("HeaderEndToken", arrayOf(type(TYPEID_CHAR), (int)strlen(HEADER_END_TOKEN)));
			}

			// (!) somehow make sure the data begins on block's boundary - refer to the host's data for that
			if (p0)
			{
				POSITION _cp(_X(B1.BytesInBlock));//adjusted
				Block B2;
				for (POSITION oBlk(sizeof(FirstBlock)); 
					p0->dataAt(_cp + oBlk, sizeof(B2), (PDATA)&B2) == sizeof(B2);
					oBlk += sizeof(Block), _cp += _X(B2.BytesInBlock))
				{
					if (_cp >= mr.cp())
					{
						mr.setcp(_cp);
						break;
					}
				}
			}

			OFF_t size0(mr.size());
			size_t num_points(size_t(size0 - mr.cp()) / 8);//8 : sizeof subblock (1 point)
			size_t size;// = num_points - ((num_swept_variables * num_sweeps)) / (h.num_vectors);
			bool bComplex(meFormat == HS_AC);
			if (bComplex)
				size = num_points / ((h.num_vectors - h.num_probed - 1) * 2 + ((h.num_probed == 0 && h.num_vectors == 1) ? 1 : h.num_probed));
			else
				size = (num_points - (num_swept_variables * num_sweeps)) / (h.num_vectors);

			OpType_t eTypeScale(h.bScaleIsFloat ? TYPEID_FLOAT : TYPEID_DOUBLE);
			OpType_t eTypeSignal(h.bSignalIsFloat ? TYPEID_FLOAT : TYPEID_DOUBLE);
			OpType_t eTypeSweep(eTypeSignal);
			HTYPE pTypeSignal;
			
			if (bComplex)
			{
				if (mr.NewScope("COMPLEX"))
				{
					SAFE_SCOPE_HERE(mr);
					mr.declField("re", type(eTypeSignal));
					mr.declField("im", type(eTypeSignal));
				}
				pTypeSignal = type("COMPLEX");
			}
			else
				pTypeSignal = type(eTypeSignal);

			
			//declare a new structure
			if (mr.NewScope("SAMPLE"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("scale", type(eTypeScale));
				int n(h.num_vectors - 1 - h.num_probed);
				if (n > 0)
					mr.declField("signals", arrayOf(pTypeSignal, h.num_vectors - 1 - h.num_probed), ATTR_COLLAPSED);
				if (h.num_probed > 0)
					mr.declField("probes", arrayOf(type(eTypeSignal), h.num_probed), ATTR_COLLAPSED);//always real
			}

			if (mr.NewScope(mr.declField("RealData")))
			{
				SAFE_SCOPE_HERE(mr);
				for (int i(0); mr.cp() < mr.size(); i++)
				{
					char buf[32];
					sprintf(buf, "SWEEP_#%d", i);
					if (mr.NewScope(mr.declField(buf, ATTR_COLLAPSED)))
					{
						SAFE_SCOPE_HERE(mr);
						mr.installNamespace();
						if (num_swept_variables > 0)
						{
							if (num_swept_variables > 1)
								mr.declField("sweep_values", arrayOf(type(eTypeSweep), num_swept_variables - 1), ATTR_COLLAPSED);
							else
								mr.declField("sweep_value", type(eTypeSweep));
						}

						for (int j(0); mr.cp() < mr.size(); j++)
						{
							sprintf(buf, "sample_#%d", j);
							mr.declField(buf, type("SAMPLE"), ATTR_COLLAPSED);
							if (mr.cp() < mr.size())
							{
								if (eTypeScale == TYPEID_FLOAT)
								{
									DECLDATA(float, aTok);
									float tok(_X(aTok.get()));
									static const float END_OF_SWEEP = 1e+030f;
									if (tok == END_OF_SWEEP)
									{
										sprintf(buf, "END_OF_SWEEP_#%d", i);
										mr.declField(buf, type(eTypeScale));
										break;
									}
									if (fabsf(tok) > END_OF_SWEEP)//same sanity check
										break;
								}
								else
								{
									DECLDATA(double, aTok);
									double tok(_X(aTok.get()));
									static const double END_OF_SWEEP = 1e+030;
									if (tok == END_OF_SWEEP)
									{
										sprintf(buf, "END_OF_SWEEP_#%d", i);
										mr.declField(buf, type(eTypeScale));
										break;
									}
									if (fabs(tok) > END_OF_SWEEP)//same sanity check
										break;
								}
							}
						}
					}
				}
			}
		}
	}

protected:

	HTYPE type(OpType_t i){ return mr.type(i); }
	HTYPE type(HNAME n){ return mr.type(n); }
	HTYPE arrayOf(HTYPE t, unsigned n){ return mr.arrayOf(t, n); }

	void createStructures()
	{
	}

};


///////////////////////////////////////////////////////


//TR0
class CDynamicType_TR0 : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_TR0"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TR0 a(r, HS_TR);
		a.preformat(nSize);
	}
};



//pre-processed TR0
class CDynamicType_TRZ : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_TRZ"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TRZ a(r, HS_TR);
		a.preformat(nSize);
	}
};

//SW0
class CDynamicType_SW0 : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_SW0"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TR0 a(r, HS_SW);
		a.preformat(nSize);
	}
};

//pre-processed SW0
class CDynamicType_SWZ : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_SWZ"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TRZ a(r, HS_SW);
		a.preformat(nSize);
	}
};

////////////////////////////////////////////////////////////////// AC0

class CDynamicType_AC0 : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_AC0"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TR0 a(r, HS_AC);
		a.preformat(nSize);
	}
};


//pre-processed AC0
class CDynamicType_ACZ : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_ACZ"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TRZ a(r, HS_AC);
		a.preformat(nSize);
	}
};

//////////////////////////////////////////////////////////////// FT0

class CDynamicType_FT0 : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_FT0"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TR0 a(r, HS_FT);
		a.preformat(nSize);
	}
};

//pre-processed FT0
class CDynamicType_FTZ : public I_FormatterType
{
protected:
	virtual const char *name() const { return "HSPICE_FTZ"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_TRZ a(r, HS_FT);
		a.preformat(nSize);
	}
};

DECLARE_FORMATTER(CDynamicType_TR0, HSPICE_TR0);
DECLARE_FORMATTER(CDynamicType_SW0, HSPICE_SW0);
DECLARE_FORMATTER(CDynamicType_AC0, HSPICE_AC0);
DECLARE_FORMATTER(CDynamicType_AC0, HSPICE_FT0);

DECLARE_FORMATTER(CDynamicType_TRZ, HSPICE_TRZ);
DECLARE_FORMATTER(CDynamicType_SWZ, HSPICE_SWZ);
DECLARE_FORMATTER(CDynamicType_ACZ, HSPICE_ACZ);
DECLARE_FORMATTER(CDynamicType_ACZ, HSPICE_FTZ);

void HSPICE_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("RC_ICO"));
	rMain.RegisterFormatterType(_PFX("HSPICE_TR0"));
	rMain.RegisterFormatterType(_PFX("HSPICE_SW0"));
	rMain.RegisterFormatterType(_PFX("HSPICE_AC0"));
	rMain.RegisterFormatterType(_PFX("HSPICE_FT0"));

	rMain.RegisterFormatterType(_PFX("HSPICE_TRZ"));
	rMain.RegisterFormatterType(_PFX("HSPICE_SWZ"));
	rMain.RegisterFormatterType(_PFX("HSPICE_ACZ"));
	rMain.RegisterFormatterType(_PFX("SILOS_SIM"));
}

