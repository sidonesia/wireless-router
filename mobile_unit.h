#ifndef __MOBILE__UNIT__
#define __MOBILE__UNIT__

#include <pthread.h>
#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "table.h"
#include "router.h"


#define LIFETIME 	  15
#define COA_DISABLED  50

/*
 * Defines the dual interface on the mobile host
 */
typedef struct
{
	int CoA_IFace;
	int HMA_IFace;
}
MUnitINTERFACE;

/*
 * Defines the Mobile Units home agent details
 */
typedef struct
{
	/* This is all the routers details HomeAgent Router */
	char MobUnitHomeAgentHost[32];
	int  MobUnitHomeAgentPort;
	int  MobUnitHomeAgentNwk;
	int  MobUnitRange;
}
MUnitHOMEAgent;


/*---------------------------------------------------------
 * Defines the current state of the mobile unit
 * at this present time
 ----------------------------------------------------------*/
typedef struct
{
		/* if true use the home address Not CoA*/
		BOOLEAN AmIAtHomeNow; 
		BOOLEAN HaveIRegistered; /* at HA */
		BOOLEAN AmIRegistered;   /* at FA */
		BOOLEAN WasPreviouslyRegs;
		BOOLEAN AmIMoving;

		/*
		 * These 2 variables determine wheter or not 
		 * we should keep the unterfaces up or not
		 *
		 * */
		BOOLEAN HomeInterfaceUp;
		BOOLEAN COAInterfaceUp;
		
		int tmpAddrTries;
		/* the number of times an icmp was recvived and used for registration but was unsuccseful*/
		int waitFailedCount;
		/* the amount of time the mu is registered on the network */
		int LifeTimeOfReg;
		/* this will be either 1,2,3,4,5 in the context of this assignment */
		int FrgnNetwork; 
		/* this will always be 1000  it is the mutiplier */
		int FrgnNetworkRange; 
		/* this is the specific port on which mu listens random */
		/* care of address port !! */
		int CurrentNetworkPort;

		/*if we are at the home network, 
		this is the port we send through*/
		int HomeAddressPKTSProcessed;

		/*if we are at the foreign network, 
		this is the port we send through*/
		int CareOfAddressPKTSProcessed;
		int previousCareOfAddress;
		
		/*
			the amount of time to stay at this
			network before moving to the next one
		 */
		int StayOnThisNetworkTime;
		int RegisterTime;
		int HomeAddressRouterPort;
		int FrgnAddressRouterPort;

		/* 7000 8000 9000 whatever !*/
		int RouterRanges; 
		
		char MobUnitHost[32];
		
		MUnitINTERFACE DualIf;
		MUnitHOMEAgent HAgentDetails;
}
MobileUnit_t;

/* enter globals below here */
RouterCONN_Channel*  CONNECTChanHomeAddress;
RouterCONN_Channel*  CONNECTChanCareOfAddress;
MobileUnit_t*        MUnit;
BOOLEAN              execute; 
/* enter globals above here */
//point_location PATH_OF_MU[MU_MAX_TRAVEL];

int MOBUNT_InitPath();
int MOBUNT_makeReqestPacket();
int MOBUNT_UnitMove();

int MOBUNT_SendRequestPKT
       (
		  PACKET_size_t RQST_PKT,
		  int FAgent
	   );

int MOBUNT_CallbackHA
       (
		   PACKET_send_t RawPacket
	   );

int MOBUNT_CallbackFA
       (
		   PACKET_send_t RawPacket
	   );

int MOBUNT_makeIPPacket
       (
		   int destNwk,
		   int destIf
	   );

int MOBUNT_sendIPPacket
       (
		   int Network,
		   int Interface
	   );
	   
int MOBUNT_DispatchPacketWait
	  (
	  	   int RoutersPortNumber,
		   PACKET_size_t PKT
	  );

int MOBUNT_ShutdownCoA();
int MOBUNT_ShutdownHMA();

int MOBUNT_Init(char* configFile);

/*------------------------------------------------------------ */
int   MOBUNT_SetMUsPath();
/*--------- Functions to bring up the MU's Interfaces -------- */
int   MOBUNT_BringUpAllInterfaces();
int	  MOBUNT_ReplyIPPkt(PACKET_size_t PKT);
int   MOBUNT_TriggerChangeInCoA();
int   MOBUNT_TriggerChangeInHMA();
int   MOBUNT_ReactivateHMA();
int   MOBUNT_StillRegistered();
int   MOBUNT_GetInterfaceAddressFromDHCP(char* inf);
void* MOBUNT_BringUpHomeAddress(void* param);
void* MOBUNT_BringUpCareOfAddress(void* param);
/*------------------------------------------------------------ */

int   MOBUNT_RegisterWithHomeAgent();
int   MOBUNT_RegisterWithFrgnAgent();
int   MOBUNT_DeRegisterWithFrgnAgent();
#endif

