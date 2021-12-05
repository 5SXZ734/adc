#include <iostream>

extern "C" 
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luahost.h"


void LUAHost_t::reportErrors(int status)
{
	if ( status != 0 ) 
	{
		std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // remove error message
	}
}

static int print(lua_State *L)
{
 int n=lua_gettop(L);
 int i;
 for (i=1; i<=n; i++)
 {
  if (i>1) printf("\t");
  if (lua_isstring(L,i))
   printf("%s",lua_tostring(L,i));
  else if (lua_isnil(L,i)==2)
   printf("%s","nil");
  else if (lua_isboolean(L,i))
   printf("%s",lua_toboolean(L,i) ? "true" : "false");
  else
   printf("%s:%p",luaL_typename(L,i),lua_topointer(L,i));
 }
 printf("\n");
 return 0;
} 

LUAHost_t::LUAHost_t()
{
	L = lua_open();

	//luaopen_io(L);
	luaopen_base(L);
	//luaopen_table(L);
	//luaopen_string(L);
	//luaopen_math(L);
	//luaopen_loadlib(L);
	lua_register(L,"print",print); 
}

LUAHost_t::~LUAHost_t()
{
	lua_close(L);
}


void LUAHost_t::exec( const char * file )
{
	std::cerr << "-- Loading file: " << file << std::endl;

//	if (luaL_dofile(L,NULL)!=0) fprintf(stderr,"%s\n",lua_tostring(L,-1)); 

	int s = luaL_loadfile(L, file);

    if ( s == 0 ) 
	{
		// execute Lua program
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
    }

	reportErrors(s);
	std::cerr << std::endl;
}




