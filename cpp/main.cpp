/*
**	Operating System
**	Project 2 - Memory Management
**	Li Yu
**	File: main.cpp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include "memManager.h"

using namespace std;

priority_queue<process, vector<process>, processComp> procQueue;

void split(string & s, vector<string> & proc_info);

int main(int argc, char * argv[]) 
{
	if( argc < 3 ) 
	{
		cerr<<"USAGE: memsim <input-file> { noncontig | first | best | next | worst }\n";
		exit(0);
	}

	// type: 0: first fit, 1: best fit, 2: next fit, 3: worst fit, 
	// 	4: non-contiguous
	map<string, int> params_map;
	params_map["first"] = 0;
	params_map["best"] = 1;
	params_map["next"] = 2;
	params_map["worst"] = 3;
	params_map["noncontig"] = 4;
	int type = params_map[string(argv[2])];

	string line;
	ifstream in(argv[1]);
	
	procQueue = priority_queue<process, vector<process>, processComp>();
	
	// read process info
	if( in ) 
	{
		// skip the first line (process number)
		getline(in, line);
		
		while( getline(in, line) ) 
		{
			vector<string> proc_info;
			split(line, proc_info);
			// proc_base consists label and size propertiesof that process
			process proc_base, proc_arr, proc_exit;			
			proc_base.label = proc_info[0][0]; 			
			proc_base.size = atoi(proc_info[1].c_str()); // e.g. 36
			
			// construct array/exit process info & put into priority queue
			for( unsigned int i = 2; i < proc_info.size(); i += 2 ) 
			{
				proc_arr = proc_base;
				proc_arr.time = atoi(proc_info[i].c_str());
				proc_arr.act = ARR; // Arrival process
				proc_exit = proc_base;
				proc_exit.time = atoi(proc_info[i+1].c_str());
				proc_exit.act = EXIT; // Exit process
				procQueue.push(proc_arr);
				procQueue.push(proc_exit);
			}
		// procQueue: A 0 ARR, D 0 ARR, B 0 ARR, C 0 ARR, E 0 ARR, 
		//	F 100 ARR, A 350 EXIT, A 400 ARR..
		}
	}
	in.close();
		
	// begin load/unload process
	memManager manager(type); // 0: contiguous, 1: non-contiguous
	unsigned int time = 0, pretime = 0;
	
	while( !procQueue.empty() ) 
	{
		while( procQueue.top().time <= time ) 
		{
			if( procQueue.empty() ) 
			{
				printf("Memory at time %d\n", time);
				manager.Display();		
				cout<<"All done.\n";
				exit(0);
			}
	
			process proc = procQueue.top();
			cout<<proc.label<<" ";
			if( proc.act == EXIT ) 
			{
				if( type != 4 ) // contiguous
					manager.Unload(proc.label);
				else 			// non-contiguous
					manager.Unload(proc.label, 1); 
				procQueue.pop();
			} 
			else 
			{
				if( type != 4 ) // contiguous
					manager.Load(proc, type); 
				else 			// non-contiguous
					manager.Load(proc); 
				procQueue.pop();
			}
		}
		
		printf("Memory at time %d\n", time);
		manager.Display();
		printf("\nInput time t: ");
		pretime = time;
		if( scanf("%u", &time) != 1 ) 
		{
			cerr<<"Incorrect time\n";
			time = pretime;
			return 0;
		} 
		else if( time == 0 ) 
		{
			printf("Exit\n");
			return 0;
		} 
		else if( time < pretime ) 
		{
			printf("Can't go back\n");
			time = pretime;
		}
	}
	return 0;
}

// split line by ' ' or '/t', store elements in proc_info
void split(string & line, vector<string> & proc_info) 
{
	char * cline = (char *)malloc(line.size());
	strcpy(cline, line.c_str());
	char tmp[10] = {0};
	int blank = 0, pos = 0;
	for( unsigned int i = 0; i < strlen(cline); i++ ) 
	{
		if( cline[i] == ' ' || cline[i] == '\t') 
		{ 
			blank++;
			if( blank == 1) 
			{
				proc_info.push_back(string(tmp));
				pos = 0;
				bzero(tmp, 10);
			} 
			else
				continue;
		} 
		else
		{ // come to a new word, blank set to 0
			tmp[pos++] = cline[i];
			if( blank != 0 )
				blank = 0;
			if( i == strlen(cline) - 1 )
				proc_info.push_back(string(tmp));
		}
	}
}
