
#include <stdio.h>
#define TEST_BEGIN(a)	namespace __test##a {
#define TEST_END(a) }

typedef unsigned int	DWORD;

TEST_BEGIN(1)
struct a { char _0[0xC]; char _0C; };
struct b
{
	a* _0;
	void test()
	{
		b *_this(this);
		if (this->_0->_0C & 3)
			if (*((char *)this->_0 + 0xC) & 3)
				if (*((char *)*(a **)((char *)this + 0) + 0xC) & 3)
					if (*((char *)*(a **)((char *)*&_this + 0) + 0xC) & 3)
					{
					}
	}
};
TEST_END(1)

//////////////////////////////////
TEST_BEGIN(2)
void func1(DWORD a)
{
	printf("func1\n");
}
struct a_t { float s; };
void func2(a_t *p)
{
	func1((DWORD)&p->s);
}
void main()
{
	unsigned a(0), c(4);
	//a++;
	short b(1);
	if (*(short *)&++a < b)
		printf("1");
	if (*(short *)&(a = a + 1) < b)
		printf("2");
	if ((a+=3)*a*(a+=2)*(a+=1) < 4)//check order of calculation: left to right
		printf("3");
}
TEST_END(2)

///////////////////////////
TEST_BEGIN(3)
void *pf = 0;
typedef unsigned *pfunc(int z);
void main(int z)
{
	*(unsigned *)(*(pfunc *)pf)(z) = 1;
	*((pfunc *)pf)(z) = 2;
	*((unsigned *(*)(int))pf)(z) = 3;
}
TEST_END(3)

TEST_BEGIN(4)
void main()
{
	int *p[4] = { 0, 0, 0, 0 };//array of pointers to int
	int *(*pp)[4] = &p;
	int *(**ppp)[4] = &pp;
	int *(***pppp)[4] = &ppp;//!cannot represent this with TYPE _t!!!!
}
TEST_END(4)

TEST_BEGIN(5)
struct S
{
	int i;
	float f;
	void func1(){ i = 1; }
	void func2(int a){ i = a; }
	bool func2(int a, float b)
	{
		i = a; f = b;
		return true;
	}
};
template<typename T, typename R>
void* void_cast(R(T::*f)(int, float))
{
	union
	{
		R(T::*pf)(int, float);
		void* p;
	};
	pf = f;
	return p;
}
// helper union to cast pointer to member
template<typename classT, typename memberT>
union u_ptm_cast {
	memberT classT::*pmember;
	void *pvoid;
};
int gi;
float gf;
void func3(int a, float b){ gi = a; gf = b; }
void main()
{
	S s;

	int n;
	void(*pobj) = (S *)&s;
	n = sizeof(pobj);
	//((void (*)(int, float))pfunc)(1, 2.0);
	//((void(S::*)(int, float))p)(1, 2.0);

	S *ps(&s);
	bool (S::*x)(int, float) = &S::func2;
	//printf("%X\n", x);
	void *pfunc2 = void_cast(x);
	//printf("%X\n", pfunc2);
	typedef bool (S::*T)(int, float);
	//typedef bool *(S::*PT)(int, float);
	typedef T*	PT;

	//void *pfunc = (void *)(&S::func2);
	//n = sizeof(pfunc);


	T y =  &S::func2;
	PT z = &y;
	n = sizeof(x);
	((S*)pobj->*(x))(3, 5.1f);
	((S*)pobj->*(y))(3, 5.1f);

	bool b;
	//b = ((S*)pobj->*((T)pfunc2))(3, 5.1f);	NG
	b = ((S*)pobj->*(*((T*)&pfunc2)))(3, 5.1f);
	b = ((S*)pobj->*(*(PT)&pfunc2))(3, 5.1f);
	//b = ((S*)pobj->*(*((bool((S::*)*)(int, float)))&pfunc2))(3, 5.1f);
	//b = ((S*)pobj->*((bool(*S::)(int, float))pfunc2)(3, 5.1f);
	//b = (S*)pobj->(*((T)pfunc2)(3, 5.1f);
	//b = (S*)pobj->(*((bool(S::)(int, float))pfunc2) (3, 5.1f);

//#define P2MCALL(pobj, pfunc) ((S*)pobj->*(*(T*)&pfunc2))

}
TEST_END(5)

TEST_BEGIN(6)
struct S
{
	int i; float f;
	bool func2(int a, float b) { i = a; f = b; return true; }
};
bool gfunc(int a, float f) { return false; }
void main()
{
	// Question 1: How do you call a global function using 1 pointer?
	int *pfunc1 = (int *)&gfunc;
	bool b = ((bool (*)(int z, float zz))pfunc1)(7, 5.1f);

	// Question 2: How do you call a class member function using these pointers? (hint: fuck C++ rules)
	S s;
	double *pobj = (double *)&s;
	void *pfunc = 0;
	// If you can define a function type in C:
	typedef bool (T1)(int, float);
	// Why can't you define a class member function type?
	// This is no more than a pointer to a function with a special (__thiscall) calling convention.
	//typedef bool (S::T)(int, float);	// error C2039: 'T': is not a member of '__test6::S'
	// b = S::func2((S*)pobj, 2, 5.1f);	// pobj is in ECX ==>
	// b = ((T)pfunc)((S*)pobj, 2, 5.1f);	// =>
	// b = (S*)pobj->func2(2, 5.1f);		// =>
	// b = (S*)pobj->((T)pfunc)(2, 5.1f);	// =>

	// (!!!)
	// b = ((S*)pobj) -> (*(bool __thiscall (S::)(int, float))pfunc) (2, 5.1f);	// This illegal in C++, but that's how it's going to look like
		// error C2059 : syntax error : '('
		// error C2589 : ')' : illegal token on right side of '::'
}
TEST_END(6)

TEST_BEGIN(7)
template <typename T>
T post(T &a)
{
	T b(a);
	a++;
	return b;
}
template <typename T, typename U>
bool postcall(T a, U b)
{
	return a;
}
template <typename T, typename U>
bool precall(T a, U b)
{
	return b;
}
bool tobool(bool b){ return b; }
int toint(int n){ return n; }
void main()
{
	//int n1 = toint(8), bool b1 = tobool(true);//error C2062: type 'bool' unexpected

	int n;
	bool b;
	n = toint(8), b = tobool(true);//OK
	int n2 = toint(8), n3 = toint(7);
	double gx3(3), gx2(9);
	double ga3 = 9, ge2 = 8;

	double ta8 = 1.0f - ga3 * 0.00392157f;
	//double gc3 = gf2, tb8 = gb3 * gf2, gf2 = ta8, gb3 = ge2;//NG
	double gb3 = ge2, gf2 = ta8, tb8 = gb3 * gf2, gc3 = gf2; ge2 = tb8;

	int count(0);
	short i(0);
	do
	{
		count++;
	//} while (i++ < 2);//OK
	//} while (post(i) < 2);//OK
	//} while (postcall(tobool(i < 2), toint(i = i + 1)));//OK
	} while (precall(toint(i = i + 1), tobool(i < 2)));//OK
	//} while (i < 2, i = i + 1);//NG
	//} while ((i = i + 1) < 2);//NG
	printf("%d\n", count);
	//arguments are evaluated from right to left(!)
}
TEST_END(7)

TEST_BEGIN(8)
int f(char a, long b)
{
	
	struct IMAGE_SECTION_HEADER {
		unsigned char    a;
		union {
			DWORD   PhysicalAddress;
			DWORD   VirtualSize;
		} Misc;
		DWORD   PhysicalAddress;
		struct {
			DWORD   PhysicalAddress;
			DWORD   PointerToRawData;
			DWORD   PointerToRelocations;
			DWORD   PointerToLinenumbers;
		} y;
		unsigned short    NumberOfRelocations;
		unsigned short    NumberOfLinenumbers;
		DWORD   Characteristics;
	} z;

	z.Misc.PhysicalAddress = 0;
	z.PhysicalAddress = 0;
	z.y.PhysicalAddress = 0;

	printf("%c,%d", a, b);
	return b;
}
void main()
{
#if(0)//error C4235: nonstandard extension used : '__asm' keyword not supported on this architecture
	const char *s = "test";
	int n(10);
	__asm { mov edi, s}
	__asm { mov ecx, n}
	__asm { mov al, 0}
	__asm { repne scasb}
	__asm { mov s, edi}//edi advanced as well!
	__asm { mov n, ecx}
	printf(s);
#endif

	int(*pf)(char, long) = f;
	int(**ppf)(char, long) = &pf;
	pf('6', 6);
	(*pf)('7', 7);
	(*ppf)('8', 8);
	(**ppf)('9', 9);
	(*****ppf)('9', 9);//????

	int g; g = 9;
	int a = g, b = a * 4;
	int a1;
	a1 = 3; 

}
TEST_END(8)

void __test()
{
	__test8::main();
}