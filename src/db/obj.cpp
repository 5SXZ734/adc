#include "obj.h"


#ifdef _DEBUG
static int g_obj_created = 0;
static int g_obj_destroyed = 0;
#endif
void print_obj_stats()
{
#ifdef _DEBUG
	std::cout << "obj_created = " << g_obj_created 
		<< " obj_alive = " << g_obj_created - g_obj_destroyed
		<< std::endl;
#endif
}
//////////////////////////////
// Obj_t

int IdCheck_t::sID = 0;

IdCheck_t::IdCheck_t(int id)// *** OPs ***
	: mID(id)
{
CHECK(id == 4378)
STOP
//#ifdef _DEBUG
//g_obj_created++;
//#endif
}

IdCheck_t::~IdCheck_t()
{
CHECK(mID == 18627)
STOP
	//mID = -1;//to debug
}






#ifdef _DEBUG

/////////////////////////////////////////////////// [constructor] *** FIELDS/TYPES/GLOBS ***
void Obj_t::on_new_obj()
{
CHECKID(this, 0x286ea)
STOP
CHECKID(this, 0x1ce5b)
STOP
	g_obj_created++;
}




/////////////////////////////////////////////////// [destructor]
void Obj_t::on_kill_obj()
{
CHECKID(this, 0x34a)
STOP
CHECKID(this, 20110)
STOP
	g_obj_destroyed++;
//	mID = -1;//to debug
}

#endif







