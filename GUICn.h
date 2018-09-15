#ifndef  _CN_
#define  _CN_

#include <stdio.h>
#include <stdlib.h>
#include <graphapp.h>
#include "common_include.h"
#include "table.h"
#include <pthread.h>
#include "mip_protocol.h"
#include "CN_terminal.h"



typedef struct
{
	App*     applic;
	Window*  gui_win;
	
	Control* sendButton;
	
	Control* NwkLabel;
	Control* InfLabel;
	Control* IPLabel;
	Control* CNsLabel;
	
	Control* NwkText;
	Control* InfText;
	Control* IPText;
	Control* CNsText;

	Control* MSGList;
	Control* TABPage;

	Graphics* topLineG;
	Graphics* botLineG;

	Point    topLineLeft;
	Point    topLineRight;

	Point    BotLineLeft;
	Point    BotLineRight;
	
	int      ListCounter;
	
	pthread_t sendPktThread;
	pthread_t startIfThread;

}GUICn;

char* MSGList[200];
char* config_file;

GUICn GC;


int   initStructuresCN();
int   createCnGui(int argc,char** argv);

void* sendIpPacketWrapper(void* param);
void   sendIpPacket(Control* C);

int   startNetworkInterface();
void* startNetworkInterfaceWrapper(void* param);

int   addToMsgList(char* msg,PACKET_size_t PKT);

#endif
