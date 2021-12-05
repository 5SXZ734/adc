#ifndef __LUAHOST_H__
#define __LUAHOST_H__

extern "C" struct lua_State;

class LUAHost_t
{
	lua_State *	L;
public:
	LUAHost_t();
	~LUAHost_t();

	void exec(const char * file);
	void reportErrors(int status);
};


#endif//__LUAHOST_H__

