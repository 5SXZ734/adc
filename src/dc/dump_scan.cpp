#include "dump_scan.h"
#include "interface/IADCGui.h"

//IDumpScanner_t

int IDumpScanner_t::scan(const char* line0)
{
	const char* p(line0);
	if (!p || !*p)
		return -1;

	int len = 0;
	bool bBreak = false;

	while (1)
	{
		unsigned char c(*p);
		p++;

		if (!c)
		{
			OnLineEnd();
			//mos << '\0';
			break;
		}

		switch ((unsigned)c)
		{
		case adcui::SYM_LINEFEED:
			OnLineEnd();
			//mos << '\0';
			bBreak = true;
			break;
		case adcui::SYM_TAB:
			OnTab();
			//dumpTab();
			//mos << c;
			break;
		case adcui::SYM_COLOR:
			OnColor(*p);
			//PushColor(*p);
			p++;
			break;
			/*		case adcui::SYM_FONT:
						OnFont(*p);
						//PushFont(*p);
						p++;*/
						//mos << c;
						//c = *p++;
						//mos << c;
			break;
		case adcui::SYM_WSTRING:
		{
			uint16_t ulen(*(uint16_t*)p);
			p += sizeof(uint16_t);
			OnWString((const wchar_t*)p, ulen);
			p += sizeof(wchar_t) * ulen;
			break;
		}
		case adcui::SYM_FUNCDECL://as in prototyping
			OnFuncDecl(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_FUNCDEF://as in func definition
			OnFuncDefinition(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_STUBINFO:
			OnStubInfo(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_FLDDECL:
			OnFieldDecl(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_FLDDECL0:
			OnFieldDecl(*(CFieldPtr*)p, true);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_FLDDEF:
			OnFieldDef(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
			/*case adcui::SYM_FLDINST:
				OnFieldInst(*(CFieldPtr *)p);
				p += sizeof(CFieldPtr);
				break;*/
		case adcui::SYM_GLBDECL:
			OnGlobDecl(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_GLBDEF:
			OnGlobDef(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_GAP:
			OnFieldGap(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_FLDREF:
			OnFieldRef(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_CONSTREF:
			OnConstRef(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_IMPFLDREF:
			OnImpFieldRef(*(CFieldPtr*)p);
			p += sizeof(CFieldPtr);
			break;
		case adcui::SYM_PATHREF:
			OnLabelDecl(*(PathPtr*)p);
			p += sizeof(PathPtr);
			break;
		case adcui::SYM_OPREF:
			OnOp(*(OpPtr*)p);
			p += sizeof(OpPtr);
			break;
		case adcui::SYM_IMPTYPEDEF:
			OnStrucDecl(*(CTypePtr*)p, true);
			p += sizeof(CTypePtr);
			break;
		case adcui::SYM_TYPEDEF:
			OnStrucDecl(*(CTypePtr*)p, false);
			p += sizeof(CTypePtr);
			break;
		case adcui::SYM_STRUCEND:
			OnStrucEnd(*(CTypePtr*)p);
			p += sizeof(CTypePtr);
			break;
		case adcui::SYM_TYPEREF:
			OnTypeRef(*(CTypePtr*)p, false);
			p += sizeof(CTypePtr);
			break;
		case adcui::SYM_IMPGLB://as in func definition
			OnImpGlobDecl(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
		case adcui::SYM_IMPCLSGLB:
			OnImpClsGlob(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			break;
			/*case adcui::SYM_EXTCLSFLD:
			{
				OnExtClsField(*(ExpFieldInfo_t *)p);
				p += sizeof(ExpFieldInfo_t);
			}
			break;*/
		case adcui::SYM_VFUNCDECL:
		{
			CGlobPtr g(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			int off(*(int*)p);
			p += sizeof(int);
			OnVFuncDecl(g, off);
			break;
		}
		case adcui::SYM_VTBLDECL:
		{
			CGlobPtr g(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			int off(*(int*)p);
			p += sizeof(int);
			OnVTableDecl(g, off);
			break;
		}
		case adcui::SYM_IMPVTBLDECL:
		{
			CGlobPtr g(*(CGlobPtr*)p);
			p += sizeof(CGlobPtr);
			int off(*(int*)p);
			p += sizeof(int);
			OnImpVTableDecl(g, off);
			break;
		}
		default:
			OnDefault(c);
			//if ((int)c < 0x20)
			//	mos << "<ERROR>";
			//else
			//	mos << c;
			break;
		}

		if (bBreak)
			break;
	}

	return (int)(p - line0);
}

