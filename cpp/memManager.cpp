/*
**	Operating System
**	Project 2 - Memory Management
**	Li Yu
**	File: memManager.cpp
*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include "memManager.h"

using namespace std;

////////////////////////////////
// Desc: sort entries in priority queue in order of time and act
bool processComp::operator() (const process & lhs, const process & rhs) 
{
	if( lhs.time != rhs.time ) 
		return ( lhs.time > rhs.time );
	else
		return ( lhs.act > rhs.act ); // exit put before arrive
}


////////////////////////////////
// Desc: manage the memory allocation
memManager::memManager(int type = 0) 
{
	Fill(pool, '.', pool_size);
	
	if( type < 4 ) 
	{ // Contiguous cases
		used_pid = defrag = defragDone = 0;
		Fill(pool, '#', os_size);
		pFListHeader = new freeList;
		pFListHeader->addr = pool + os_size;
		pFListHeader->size = pool_size - os_size;
		pFListHeader->next = NULL; 
		pFListLast = NULL;
		pFListMax = pFListHeader;
	} 
	else if( type == 4 ) 
	{ // Non-contiguous case
		memPageTable = vector<char>((int)ceil((double)pool_size/page_size), 0);
		// pi: page index, page_qty: page quantity
		int fi = 0, page_qty = (int)ceil((double)os_size / page_size);
	
		while( page_qty > 0 ) 
		{
			if( memPageTable[fi] == 0 ) 
			{
				memPageTable[fi] = 1;
				Fill('#', fi);
				page_qty--;
			}
			fi++;
		}
	}
}


memManager::~memManager() {}


// Desc: load process in the case of contiguous allocation
void memManager::Load(process & proc, int type = 0) 
{

	if( proc.size > pool_size - os_size ) 
	{
		cerr<<"Out of memory\n";
		exit(0);
	}

	freeList * pFListCur, * pFListTmp;
	pFListCur = pFListHeader;
	pFListTmp = pFListMax;
	
	switch( type ) 
	{
		case 0:	// First fit
			while( pFListCur->next != NULL &&
			pFListCur->size < proc.size ) 
			{
				pFListCur = pFListCur->next;
			}
			break;
		case 1: // Best fit 
			while( pFListCur->next != NULL ) 
			{
				if( pFListCur->size > proc.size && 
				pFListCur->size < pFListTmp->size ) 
					pFListTmp = pFListCur;
				pFListCur = pFListCur->next;
			}
				pFListCur = pFListTmp;
			break;
		case 2: // Next fit
			if( pFListLast != NULL )  // after the first time
				pFListCur = pFListTmp = pFListLast;
			while( pFListCur->size < proc.size ) 
			{
				pFListCur = pFListCur->next;
				if( pFListCur == NULL )
					pFListCur = pFListHeader;
				else if( pFListCur == pFListTmp ) // no room for the proc through a loop
					break;
			}
			pFListLast = pFListCur;		
			break;
		case 3: // Worst fit
			pFListCur = pFListMax;
			break;
		default:
			cout<<"Type unknown\n";
			exit(1);
	}

	// Defragment & test if have defragmented
	if( pFListCur->size < proc.size )
	{
		defrag = 1;
		if( defragDone == 1 )
		{
			cerr<<"Out of memory\n";
			exit(0);
		}
		cerr<<"Defragment required\n";
		Defragment();
		Load(proc, type); // Re-load the proc
	}
	
	// If return from a load() that after a defragmented(), skip the rest
	if( defrag == 1 )
		if( defragDone == 1 ) 
			return;
	
	char * addr = NULL;
	addr = pFListCur->addr;
	pFListCur->addr += proc.size;
	pFListCur->size -= proc.size;
	proc.pid = ++used_pid;
	
	procEntry entry;
	entry.label = proc.label;
	entry.addr = addr;
	entry.size = proc.size;
	procTable.push_back(entry);
	
	Fill(addr, proc.label, proc.size);

	// Test the inner load()
	if( defrag == 1 )
		if( defragDone == 0 )
			defragDone = 1;
}


// Desc: load process in the case of non-contiguous allocation
void memManager::Load(process & proc) 
{
	
// DEBUG	cout<<"Load "<<proc.label<<", "<<proc.size<<'\n';
	if( proc.size > pool_size - os_size ) 
	{
		cerr<<"Out of memory\n";
		exit(0);
	}
	
	procPage ppage;
	ppage.label = proc.label;

	// page index(logical), frame index(physical)	
	size_t pi = 0, fi = 0;
	size_t page_qty = (int)ceil((double)proc.size / page_size);
	size_t lastpage_size = proc.size % page_size;
	lastpage_size = (lastpage_size == 0) ? page_size : lastpage_size;

	while( pi < page_qty && fi < memPageTable.size() ) 
	{
		if( memPageTable[fi] == 0 ) 
		{
			// Last page may not be entirely filled
			if( pi == page_qty -1 )
				Fill(proc.label, fi, lastpage_size);
			else
				Fill(proc.label, fi);
			memPageTable[fi] = 1;
			ppage.pageMap[pi] = fi;
			pi++;
		}
		fi++;
	}
	
	if( pi < page_qty - 1 && fi == memPageTable.size() ) 
	{
		cerr<<"Out of Memory\n";
		exit(0);
	}
	
	procPageTable.push_back(ppage);
}


// Desc: unload process in the case of contiguous allocation
void memManager::Unload(char label) 
{
	vector<procEntry>::iterator it;
	entryFind eFind(label);
	it = find_if(procTable.begin(), procTable.end(), eFind);
	
	if( it == procTable.end() ) 
	{
		cerr<<"Process not found\n";
		exit(0);
	}
	
	freeList * pFListCur, * pFListPre, * pFListTmp;
	pFListCur = pFListPre = pFListHeader;
	while( pFListCur != NULL && pFListCur->addr < it->addr) 
	{
		pFListPre = pFListCur;
		pFListCur = pFListCur->next;
	} // Pre is the closest freelistPointer upper than it's addr, or is theader if none is upper than it's addr
	
	if( pFListPre->addr > it->addr ) 
	{ // it is upper than header
		pFListTmp = pFListHeader;
		pFListHeader = new freeList;
		pFListHeader->addr = it->addr;	// (freeList *)&it. Is that OK?
		pFListHeader->size = it->size;
		pFListHeader->next = pFListTmp;
	} 
	else 
	{
		pFListTmp = new freeList;
		pFListTmp->addr = it->addr;
		pFListTmp->size = it->size;
		pFListTmp->next = pFListPre->next; // * DEBUG point
		pFListPre->next = pFListTmp;
	}

	Fill(it->addr, '.', it->size);
	procTable.erase(it);
	MergeFList();
}


// Desc: unload process in the case of non-contiguous allocation
void memManager::Unload(char label, int nonCon) 
{	
	vector<procPage>::iterator it;
	procPageFind ppFind(label);
	it = find_if(procPageTable.begin(), procPageTable.end(), ppFind);
	
	if( it == procPageTable.end() ) 
	{
		cerr<<"Process not found\n";
		exit(0);
	}
	int map_size = it->pageMap.size();

	for( int i = 0; i < map_size; i++ ) 
	{
		Fill('.', it->pageMap[i]);
		memPageTable[it->pageMap[i]] = 0;
	}
}


// Desc: load process in the case of contiguous allocation
void memManager::Fill(char * addr, char label, int size) 
{	
	for( int i = 0; i < size; i++ )
		*addr++ = label;
}


// Desc: load process in the case of non-contiguous allocation
void memManager::Fill(char label, int pi, int size) 
{
	char * addr = pool + pi * page_size;
	for( int i = 0; i < size; i++ )
		*addr++ = label;
}


// Desc: Every time a process is unloaded, check if new free space
// 	can be mergered with other free spaces
void memManager::MergeFList() 
{
	defrag = defragDone = 0;
	freeList * pFListCur = pFListHeader;
	freeList * pFListTmp = NULL;
	while( pFListCur->next != NULL ) 
	{
		pFListTmp = pFListCur->next;
		if( pFListCur->addr + pFListCur->size == pFListTmp->addr )
		{
			pFListCur->size += pFListTmp->size;
			pFListCur->next = pFListTmp->next;
			delete(pFListTmp);
			pFListTmp = NULL;
		} 
		else
			pFListCur = pFListTmp;
	}
	
	// Find the max node in free list
	pFListCur = pFListHeader;
	while( pFListCur->next != NULL ) 
	{
		if( pFListCur->size > pFListMax->size )
			pFListMax = pFListCur;
		pFListCur = pFListCur->next;
	}
}


// Desc: defragment the memory pool
void memManager::Defragment() 
{
	
	cout<<"Performing defragmentation...\n";
	
	// Sort procTable in the order of procEntry.addr
	sort(procTable.begin(), procTable.end(), entryComp);
	
	vector<procEntry>::iterator it1 = procTable.begin(), it2 = it1 + 1;
	int proc_count = 0;
	
	if( it1->addr > pFListHeader->addr )
		it1->addr = pFListHeader->addr;

	while( it2 != procTable.end() ) 
	{
		it1 = it2 -1;
		if( it1->addr + it1->size < it2->addr ) 
		{
			proc_count++;
			it2->addr = it1->addr + it1->size;
		}
		it2++;
	}
	
	// Reset the memory display
	Fill(pool + os_size, '.', pool_size - os_size);
	for( it1 = procTable.begin(); it1 < procTable.end(); it1++ )
		Fill(it1->addr, it1->label, it1->size);
	
	// Reset the free list
	freeList * pFListTmp, * pFListCur = pFListHeader->next;
	while( pFListCur != NULL ) 
	{
		pFListTmp = pFListCur;
		pFListCur = pFListCur->next;
		delete pFListTmp;
	}
	it1--; // Get the last procEntry in vector in order
	pFListHeader->addr = it1->addr + it1->size;
	pFListHeader->size = pool_size - (pFListHeader->addr - pool);
	pFListHeader->next = NULL;
	pFListMax = pFListLast = pFListHeader;
	
	cout<<"Defragmentation completed.\n";
	printf("Relocated %d processes to create a free memory block of %d units (%4.2f%% of total memory)\n", 
		proc_count, (int)pFListHeader->size, (double)((double)pFListHeader->size/pool_size * 100));
}


// Desc: display the current memory allocation
void memManager::Display() 
{
	int i, j;
	char * addr = pool;
	for( i = 0; i < pool_size / line_size; i++ ) 
	{
		for( j = 0; j < line_size; j++ )
			printf("%c", *addr++);
		printf("\n");
	}
}


// Desc: TEST use. List all free list
void memManager::ShowFList(string method) 
{
	cout<<"-----"<<method<<"-----\n";
	freeList * pFListCur = pFListHeader;
	int i = 1;
	while( pFListCur != NULL ) 
	{
		if( i > 10 )
			break;
		printf("%d FList node.addr: %d, .size: %d\n", i++, pFListCur->addr -pool, pFListCur->size);
		pFListCur = pFListCur->next;
	}
}
