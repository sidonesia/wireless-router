/*
 * This file is used to represent all the
 * different types of tables that can be 
 * contained within a router.
 * 
 * SO FAR IT HOLDS A:
 *     - HomeAddress to CareOfAddress coversion table
 *     - Routing Table table;
 *     - FREE/TAKEN COA Address on the router
 *     - Mobile Node Table (for Home Agent)
 *     - Mobile Node Table (for Foreign Agent)
 *     ___________________________
 *     WRITTEN BY : SIDNEI BUDIMAN
 *     ^^^^^^^^^^^^^^^^^^^^^^^^^^^
 */

/*
 * Must create another table that will record which Mobile units
 * are connected to which router, if it has moved where it has 
 * gone if it has gone if the timeout has finished etc
 */

#ifndef TABLE_H
#define TABLE_H

#include "common_include.h"
#include "mip_protocol.h"

/* bit of a hack but this will do for now ! */
#define MAX_ROUTERS_IN_NETWORK      20
#define MAX_MOBILE_UNITS_IN_NETWORK 15
#define MAX_CARE_OF_ADDRESS         15
#define MAX_HOME_ADDRESS 			15

/*---------------------------------------------------
  The table representing the registration of the
  mobile unit on the foreign network
 ----------------------------------------------------*/
typedef struct
{
	int CareOfAddress; /* this is the pkey */
	int HomeAddress; 
	int HomeAgentAddress;
	int MobileUnitLifeTime;
	int TimeIWasRegistered;
	int IsStillRegistered;
	int IsTimeLimitUp;
	int IsFieldBeingUsed; /* Dont forget to set this if true */
}
ForeignAgentsRegistered;

typedef struct
{
	int MAX_ARRAY_SIZE;
	ForeignAgentsRegistered FAHArray[MAX_CARE_OF_ADDRESS];
}
FARObj;

FARObj* new_FARObj();

/*
  will find the specified MU in the table
 */
ForeignAgentsRegistered
	findFAHR(FARObj* FAHR,int CoA);

/* 
   will set the MU in the ForeignAgent table, 
   requires the RQST PKT to do this 
 */
int	setFAHR(FARObj* FAHR,unsigned int PKT);
int showFATable(FARObj* FA);

/*--------------------------------------------------
  The table representing the registration of the
  mobile unit on the home network
 ---------------------------------------------------*/
typedef struct
{
	int HomeAddress; /* this is the pkey */ 
					 /* it needs the HA so that when the target is HA */
					 /* but the mobile unit is not there it can send it to te CoA*/
					 /* through the Foreign Agent */
	int CareOfAddress;
					 /* This bascailly holds the source address */
	int ForeignAgentAddress;
					 /* ID of the router whos the HA of this MU*/
	int HomeNetworkAddress;
	int TimeIWasRegistered;

	int IsStillRegistered;
	int IsStillAtHomeNwk; /* use a thread to poll the mu, to see if it still there */
	                      /* this is how we justify if the mu is still at homeor not*/
	int IsFieldBeingUsed;
}
HomeAgentsRegistered;

typedef struct
{
	int MAX_ARRAY_SIZE;
	HomeAgentsRegistered HARArray[MAX_HOME_ADDRESS];
}
HARobj;

HARobj* new_HARobj();

HomeAgentsRegistered
	findHAR(HARobj* me,int HomeAddress);

int setHAR(HARobj* me,unsigned int PKT);
int showHATable(HARobj* HA);

/*
 * This struct represents the mapping between
 * the care of address and the Home address of
 * a mobile unit. This structure will normally
 * be stored within a router 
 */
typedef struct 
{
	/*
	 * If this is set to 1 then the element
	 * is no longer valid and the values can
	 * be over written
	 */

	int AmIStillValid;
	/*
	 * Will represent the Mobile hosts
	 * Care of address and mobile hosts
	 * Home Address as a char so its easy to print
	 */
	/* maybe this should be just a char[2] instead of char* well see */ 
	char* ch_CareOfAddr;
	char* ch_HomeAddr;
	
	/*
	 * Will represent the same as above
	 * but in terms of ints so its easy
	 * To play with
	 */
	 int uint_CareOfAddr;
	 int uint_HomeAddr;
	
	/*
	 * Will represent the mobile units socket
	 * port that it is currently listening on
	 */
	void* MobileHostSocket;
}HAddrToCAddr;


/*
 * This struct will represent the routing table
 * it contains the fields that are found in a 
 * routing table.
 * This structure will commonly be found within
 * a router
 */
typedef struct
{
		/*
		 * All fields here pretty self explanitory
		 */
		int   HasFieldBeenTaken;
		 
		char chDestination[3];
		char chRoute[3];
		
		/* 
			This is the network address it will be either just
			N1,N2,N3,N4,N5,N6 since there are only 6 networks
		*/
		unsigned int intDestination;

		/* 
			this is the router number in this case it will be
			either R1,R2,R3,R4,R5
		*/
		unsigned int intRoute;
		unsigned int intDistance;

		/*
		 * socket that represents the next
		 * hop router. This is the socket 
		 * number the next hop router will
		 * be listening on
		 */
		
		// For the socket lets add 7000 to whatever the router
		// name is. we should have this in the config file though
		int  NextRouteSocket;
		char NextRouterHost[50];
		
}RoutingTable;

/*
 * This struct defines the care of address
 * that are found within a router.
 * 
 * If COA_taken is 1 that means it is taken
 * if COA_taken is 0 it means that this COA 
 * is still available
 */
typedef struct
{
	int COA;
	int COA_taken;

}CareOfAddressTable;

/* BELOW HERE ARE THE OBJECTS USED TO MODIFY THE ABOVE STRUCTURES */
/* THE ABOVE STRUCTURES ARE THE INTERNALS OF THE OBJECTS BELOW */

/********************************************************/
/* In this section we define the object that will handle
 * all operations on the HAddrToCAddr structure
 *
 * The object below is the wrapper object around this
 * structure.
 */

/* OBJECT ORIENTED C PROGRAMMING ! below here */
/* MAKES THINGS EASIER TO UNDERSTAND AS THE App gets bigger */
typedef struct 
{
		int MAX_ARRAY_SIZE;
		HAddrToCAddr HACAarray[MAX_MOBILE_UNITS_IN_NETWORK];

	    int   (*findHomeAddressBYCOA)  (void*,int);	
	    int   (*findHomeAddressBYHA)   (void*,int);	
	    int   (*findCareOfAddressBYHA) (void*,int);	
	    int   (*findCareOfAddressBYCOA)(void*,int);	
	    HAddrToCAddr  (*getHAddrToCAddrStruct) (void*,int);	

		int   (*setHomeAddress)(void*,int);
		int   (*setCareOfAddress)(void*, int);
		int   (*setHAddrToCAddrSocket)(void*,void*);

		int   (*clearAllvalues)(void*);

}HAddrToCAddrOBJ;

/*
 * will return an index pointing to the 
 * structure contain the required values
 */

// will return the HomeAddress by CareOfAddress
int findHomeAddressBYCOA(void* me,int careOfAddress);

// will return the HomeAddress by HomeAddress
int findHomeAddressBYHA(void* me,int HomeAddress);

// will return the CareOfAddress by HomeAddress
int findCareOfAddressBYHA(void* me,int HomeAddress);

// will return the CareOfAddress by CareOfAddress
int findCareOfAddressBYCOA(void* me,int HomeAddress);

// will get the actual HaddrToCaddr struct
HAddrToCAddr getHAddrToCAddrStruct(void* me,int index);

// simple set functions
int setHomeAddress(void* me,int HomeAddress);
int setCareOfAddress(void* me,int CareOfAddress);
int setHAddrToCAddrSocket(void* me,void* sockT);

/* if struct is found to be nolonger valid then 
 * we should clear all its set values
 */
int clearAllvalues(void* me);

int initHAddrToCAddrOBJ(void* me);
HAddrToCAddrOBJ* new_HAddrToCAddrOBJ();

/* ---------HAddrToCAddrOBJ DONE !---------------------- */
/*
 * The object defined below here will handle the 
 * CareOfAddressTable structure.
 * 
 * It will modify the fields within this structure
 * to either set the careOfAddress value as set or
 * The careOfAddress value as free
 *
 * Have All the Free Care Of addresses stored in
 * The routers configuration file
 */

#define FREE  0
#define INPKT 1
#define USED  2

 typedef struct
{
	 int MAX_ARRAY_SIZE;

	 CareOfAddressTable COATableArray[MAX_CARE_OF_ADDRESS];	

	 int   (*getCOAAddress)(void*,int); 
	 int   (*setCOAUsedBit)(void*,int,int); 
	 int   (*getCOAUsedBit)(void*,int); 

}CareOfAddressTableOBJ;

int getCOAAddress(void* me,int index);
int setCOAUsedBit(void* me,int ix ,int used);
int getCOAUsedBit(void* me,int index);

/* will initialise the COATableArray with NULLS */
int initCareOfAddressTableOBJ(void* me);

CareOfAddressTableOBJ* new_CareOfAddressTableOBJ();

/* ----------- CareOfAddressTableOBJ DONE ! -------------- */

/* if the next hop is direct then it is really zero */
#define DIRECT 0

typedef struct
{
		int MAX_ARRAY_SIZE;

		RoutingTable RTArray[MAX_ROUTERS_IN_NETWORK];
		
		int   (*setDestination)(void*,int );
		int   (*setDistance)(void*,int,int);
		int   (*getDistance)(void*,int);

		int   (*setNextHopRoute)(void*,int,int);
		int   (*getNextHopRoute)(void*,int);
		int   (*setNextRouteSocket)(void*,int,int);
		int   (*getNextRouteSocket)(void*,int);
		int   (*setNextRouterHost)(void*,int,char*);
		char* (*getNextRouterHost)(void*,int);
		int   (*showRoutingTable)(void*);

} RoutingTableOBJ;

// All of these structs will be called from the
// findNetworkDestination.
// This will return the primary key

/* Is the primary field for the routing table 
 * This will be different in all of them
 *
 * Call the [findNetworkDestination] from within the
 * other functions
 */
// if this returns null then it means that the entry does
// not exist
RoutingTable findNetworkDestination(void* me,int network_pkey);

/* Will probably be used at the beginning only when the 
 * Neighbouring routers are read from a file
 */ 
int setDestination(void* me,int network);

int setDistance(void* me,int network,int distance);
int getDistance(void* me,int network);

/* will get the next hope router*/
int setNextHopRoute(void* me,int network,int nextRouter);
int getNextHopRoute(void* me,int network);

/* 7000 + the RouterNumber is the socket address */
int setNextRouteSocket(void* me,int network,int socketRange);
int getNextRouteSocket(void* me,int network);

int setNextRouterHost(void* me,int network,char* hostOfRouter);
char* getNextRouterHost(void* me,int network);

/* will initialise the array with NULL values */
int initRoutingTableOBJ(void* me);
int showRoutingTable(void* me);

RoutingTableOBJ* new_RoutingTableOBJ();

/* ----------- RoutingTableOBJ DONE ! -------------- */

#endif
