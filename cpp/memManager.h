/*
**	Operating System
**	Project 2 - Memory Management
**	Li Yu
**	File: memManager.h
*/

#ifndef PRO_H
#define PRO_H

#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <string>

using namespace std;


///////////////////////////////////////
const int EXIT = 0;
const int ARR = 1;

typedef struct {
	int pid; // no use here. use label to mark instead
	char label;
	size_t size;
	size_t time;
	int act; // 0 exit, 1 arrive
} process;


class processComp {
public:
	bool operator() (const process & lhs, const process & rhs);
};


///////////////////////////////////////
// Pre-define findEntry
// Because it's used in memManager
class findEntry;


///////////////////////////////////////
class memManager {
	
private:
	enum { pool_size = 2400, os_size = 80, line_size = 80, page_size = 20 };
	int used_pid, defrag, defragDone;
	char pool[pool_size];

	// freelist is for contigueous memory allocation
	typedef struct freelist {
		char * addr;
		size_t size;
		struct freelist * next;
	} freeList;
	freeList * pFListHeader, * pFListLast, * pFListMax;
	
	// procEntry/procTable are for contiguous memory allocation
	typedef struct {
		char label;
		char * addr;
		size_t size;
	} procEntry;
	vector<procEntry> procTable;	
	
	// pageEntry/pageTable are for non-contiguous
	typedef struct {
		char label;
		map<int, int> pageMap;
	} procPage; 
	vector<procPage> procPageTable; // map proc to its page table
	vector<char> memPageTable; // pages status of system memory, 1: occupied, 0: free

public:

	struct entryCompare {
		bool operator()( const procEntry & lhs, const procEntry & rhs ) {
			return ( lhs.addr < rhs.addr );
		}
	} entryComp;
	
	class entryFind {
	private:
		char label;
	public:
		entryFind(char v):label(v) {}
		bool operator()(const procEntry & lhs) {
			return ( lhs.label == label );
		}
	};
	
	class procPageFind {
	private:
		char label;
	public:
		procPageFind(char v):label(v) {}
		bool operator()( const procPage & lhs ) {
			return (lhs.label == label);
		}
	};
	
	memManager(int type);
	~memManager();
	void Load(process & proc, int type);
	void Load(process & proc);
	void Unload(char label);
	void Unload(char label, int nonCon);
	void Fill(char * addr, char label, int size);
	void Fill(char label, int pi, int size = page_size);
	void MergeFList();
	void Defragment();
	void Display();
	void ShowFList(string method);
};

#endif
