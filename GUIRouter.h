#ifndef _GUI_
#define _GUI_

#include <stdio.h>
#include <stdlib.h>
#include <graphapp.h>
#include "common_include.h"
#include "table.h"
#include <pthread.h>
#include "mip_protocol.h"

#define  RT_NOT_EXIST       0
#define  RT_HOP_SAME_CHANGE 1
#define  RT_HOP_SHORTER     2

typedef struct
{
	App*     applic;
	Window * gui_win;
	Control* Button;
	Control* RTList;
	Control* MSGList;
	Control* ADList;
	Control* RIPList;
	Control* FATable;
	Control* HATable;
	
	Control* OnButton;
	Control* OffButton;
	Control* HASelectB;
	Control* FASelectB;

	Control* FAcheckbox;
	Control* HAcheckbox;
	Control* RTRStatusOn;
	Control* RTRStatusOff;
	Control* RouterID;

	Control* RTRName;
	Control* RTRStatus;
	Control* HALabel;
	Control* FALabel;

	Control* TabButton;
	
	Control* TabPage[2];

	// counter
	int RTRefreshCounter;
	int ADListCounter;
	int MSGListCounter;

	pthread_t ROUTER_THREAD;

}GUIRouter;


char* RTableList[6]; 
char* RIPList[2];
char* ADTableList[2];
char* MSGList[100];
char* FAList[MAX_CARE_OF_ADDRESS];
char* HAList[MAX_HOME_ADDRESS];
char* config_file;

GUIRouter GR;

int  initStructures();
int  createRouterGui  (int argc,char** argv);
int  addToRouterList  (RoutingTableOBJ* RT);
int  addToAdvertList  (char* ad);
int  addToMSGList 	  (char* msgs,int RTEntry,PACKET_size_t PKT);
int  addToHASpecTable (HARobj* HAO);
int  addToFASpecTable (FARObj* FA0,int nwk_id);
int  addToRIPList(int thisRouter,int frgnNtwk);

void* startRouterWrapper(void* param);
void  startRouter(Control* C);
void  stopRouter(Control* C);
void  selectTablePane(Control* ctrl);

#endif
