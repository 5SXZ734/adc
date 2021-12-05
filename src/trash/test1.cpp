
//#include "globals.h"
//#include "op.h"

#pragma

void test1(int _eax)
{
	__asm {
		clc
		stc
		mov 	eax, _eax
		neg 	eax
		sbb 	eax, eax
	}
}

void test2(int _ecx)
{
	__asm		stc
	__asm		clc
	__asm		mov 	ecx, _ecx
	__asm		sbb 	ecx, ecx
	__asm		sbb 	ecx, 0FFFFFFFFh
}


void strcpy__(void *pSrc, void *pDst)
{
	__asm 
	{
		mov		ecx, 0FFFFFFFFh
		mov		eax, 0
		mov		edi, pSrc
		repne scasb
		not 	ecx
		sub		edi, ecx

//ecx = strlen(arg_0)+1
		mov		edx, pDst
		mov		eax, ecx
		mov		esi, edi
		shr		ecx, 2
		mov		edi, edx
		repe movsd
				
		mov		ecx, eax
		and		ecx, 3
		repe movsb
	}
//memcpy(pDst, pSrc, ecx);
}

const short var_68 = 1;
short var_98;
int _0CE4h = -90;


int t1,t2,t3,t4;
double varx_57 = (double)0x1000000020000000;
double varx_53 = (double)0x3000000040000000;
int p;
char pc = 2;
int (*ar)[5];

char *ss2 = "fuck?";
const char ss[47] = "\x066\x075\0\0\x063\x06B\xaa";
test()
{
	test1(0);
	__asm{
//		mov		eax, 2
//		mov		eax, offset ar
//		mov		ecx, offset [eax*4]
	}

//	p = *((int *)0x400322);

	strchr(ss, 'e');

	*(int *)((char *)&varx_57 + 4) = *(int *)((char *)&varx_53 + 4);
 	const short *j = &var_68;
	int edx = (int)(((__int64)0x55555556 * (__int64)_0CE4h) >> 0x20);
	_0CE4h = edx + ((unsigned)edx >> 0x1F);
	return 0;
};



