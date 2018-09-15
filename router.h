#ifndef ROUTER_H
#define ROUTER_H

#include <pthread.h>
#include <semaphore.h>
#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "table.h"
#include "fileIO.h"
#include "fileIOUpdate.h"
#include "RIPAlgorithm.h"
#include "GUIRouter.h"


#define  WAIT_PERIOD_B4_SEND 1
#define  FAILED_LIMIT        9
#define  LIFE_TIME           15
#define  F_AGENT             1
#define  H_AGENT             2
				/* represents the sending of ads to the network */
				/* since 15 is the max COA and HA well make the NWK_BRD_CST 15 */
#define  NETWORK_BROADCAST   15 
#define  MAX_INPKT_COA       3
#define  RIP_MAX_HOP         12

  	/* Routing Table Structure */	
	/* Special Agent Structure */
	/* A table which contains the available address for care of address */

/*-------------------------------------------------------------------------*/

RoutingTableOBJ*      	ROUTERTable;
RouterCONN_Channel*   	CONNECTChan;
CareOfAddressTableOBJ* 	COATable;
FARObj*					FAMobUnits;
HARobj*					HAMobUnits;
/*--------------------REPRESENTS THE ROUTER -------------------------------*/
typedef struct
{
		int failed_count;
}update_cnt;

typedef struct
{
	int       intRouterPacketsProcessed;
	int       intRouterID;
	int       intRouterRecvPort;

	BOOLEAN   sendRoutingTable;
	pthread_t tableDispatchThreadID;
	pthread_t icmpDispatchThreadID;
	sem_t	  update_sem;
	
	char      charHostRouter[50];
    char      charRouterID[3];

	BOOLEAN   BOOLIsRouterHomeAgent;
	BOOLEAN   BOOLIsRouterFrgnAgent;
	
	/* counter corresponds to the number of faileds per entry
	 * if the faileds reaches the FAILED LIMIT for each entry
	 * in the table then we slow down the table in sending RIP PKTS
	 * This means the routers have converged
	 */
	update_cnt FAILED_CNT[MAX_ROUTERS_IN_NETWORK];
} Router;
/*-------------------------------------------------------------------------*/
/* not used but just keep it here */
typedef struct
{
	PACKET_size_t PKT;
	int 		  whichNetworkToDispatchTo;
	int 		  dispatchFromNowTime;
}ThreadData;
/*-------------------------------------------------------------------------*/
Router* ThisRouter;
/*------------------------ICMP ADVERTISE PACKET FUNCTIONS------------------*/

int   ROUTER_advertiseICMPNetwork();
int   ROUTER_advertiseICMPPeriodic();
void* ROUTER_advertiseAsICMPNetworkWrapper(void* param);
int   ROUTER_getFreeCareOfAddress();
PACKET_size_t ROUTER_makeIcmpPackets();

/*-------------------------------------------------------------------------*/

int ROUTER_ShowTheFATable();
int ROUTER_ShowTheHATable();
int ROUTER_encapsulateDataPKT();
int ROUTER_init(char* this_rtr_cfgs);
int ROUTER_Callback(PACKET_send_t RawPacket);

/*------------------------RIP UPDATE PACKET FUNCTIONS----------------------*/
int ROUTER_UpdateMyRoutingTable
			(
				PACKET_size_t PKT,
				RoutingTableOBJ* RTObj
			);

BOOLEAN ROUTER_KeepSendingTable();
int   ROUTER_updateFailedCounter(int network,BOOLEAN failed);
int   ROUTER_sendRoutingTableTo(int NetworkID);
int   ROUTER_sendRoutingTableToNeigbour();
int   ROUTER_sendRoutingTablePeriodic();
int   ROUTER_compileRoutingTable( PACKET_size_t* PKTS_TO_SEND );         
int   ROUTER_DetermineConnectivity();
PACKET_size_t 
	  ROUTER_makeRipPackets(RoutingTable RT);
void* ROUTER_sendRoutingTableWrapper(void* param);

/*-----------------------ROUTER INTERFACE FUNCTIONS-------------------------*/
int ROUTER_bringUpSocketIntf();
int ROUTER_DispatchPacketDontWait
	  (
		PACKET_size_t PKT,
		int whichNetworkToDispatchTo,
		int dispatchFromNowTime
	  );

void* ROUTER_DispatchWrapper(void* thdData);
int   ROUTER_DispatchPacketWait
      (
		PACKET_size_t PKT,
		int whichNetworkToDispatchTo 
	  );
/*---------------------ROUTER INIT FUNCTIIONS--------------------------------*/

int     ROUTER_InitRoutingTableFromConfigFile();
int     ROUTER_InitThisRouter(char* router_configuration);
int     ROUTER_TableAssignCareOfAddress();
Router* ROUTER_newROUTER(char* configuration);

/*------------------------IP RELATED FUNCTIONS--------------------------------*/
int 	ROUTER_SendIPPKTToMU();
int 	ROUTER_makeIPPKT();
int     ROUTER_TunnelIPPKT();

/*-------------MOST OF OF THE REGISTRATION FUNCTIONS--------------------------*/
int 	ROUTER_RegisterMOBUNITAsHomeHere();
int 	ROUTER_RegisterMOBUNITAsFrgnHere();
int 	ROUTER_DeRegisterMOBUNITAsFrgnHere(PACKET_size_t REQ_PKT);
int     ROUTER_DoRouting(PACKET_size_t PKT,int PKT_TYPE);
int 	ROUTER_sendPKTToMobileUnit(PACKET_size_t PKT,int PKT_TYPE);

int     ROUTER_CheckIsMUnitStillValidAtFAStart();
int     ROUTER_CheckIsMUnitStillValidFA();
void*   ROUTER_CheckIsMUnitStillValidAtWrapper(void* param);

PACKET_size_t	
		ROUTER_PrepareREPLYPKTForFA(PACKET_size_t RQST_PKT);
/*------------------------------------------------------------------------------*/
#endif 
