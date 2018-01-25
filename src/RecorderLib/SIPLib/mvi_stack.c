#include "mvi_stack.h"
#include "mvi_log.h"

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
BOOL stack_pop_fifo(int* pbegin,int* pend,int* pmax,int* pnow)
{
	int tek;
	(*pnow) = (*pend);
	tek = (*pend);
	tek++;
	if ( tek == (*pmax) )  tek = 0;	
	if ( tek == (*pbegin) ) return FALSE;
	(*pend) = tek;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
BOOL stack_push_fifo(int* pbegin,int* pend,int* pmax,int* pnow)
{
	if ( (*pbegin) == (*pend) ) return FALSE;
	(*pnow) = (*pbegin);
	(*pbegin) ++;
	if ( (*pbegin) == (*pmax) ) (*pbegin) = 0;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
BOOL stack_check_fifo(int* pbegin,int* pend,int* pmax,int* pnow)
{
	(void)pmax; 
	(void)pnow; 

	if ( (*pbegin) == (*pend) ) return FALSE;
	(*pnow) = (*pbegin);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
BOOL stack_pop_lifo(int* pmax,int* pnow)
{
	(void)pmax; 

	if ( !(*pnow) ) return FALSE;
	(*pnow)--;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
BOOL stack_push_lifo(int* pmax,int* pnow)
{
	(*pnow) ++;
	if ( (*pnow) >= (*pmax) ) { (*pnow) --; return FALSE; }
	return TRUE;
}


