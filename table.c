/*
 * All the code in this file is pretty self explanitory
 * All this file does is implement the various tables
 * that exist in the router.
 * 
 * This tables include
 * - routeing tables
 * - HA tables
 * - FA tables
 * - Free COA tables 
 *
 * This file just implements functions to access the
 * tables and get the required data
 */

#include "table.h"

/*----------------------------------------------------------------*/
FARObj* new_FARObj()
{
	int ix = 0;
	FARObj* FO = (FARObj*)malloc(sizeof(FARObj));
	FO->MAX_ARRAY_SIZE = MAX_CARE_OF_ADDRESS;
	for(ix =0;ix < MAX_CARE_OF_ADDRESS;ix++)
	{
		FO->FAHArray[ix].IsFieldBeingUsed = FALSE;
		FO->FAHArray[ix].CareOfAddress     = ERR;
		FO->FAHArray[ix].HomeAddress 	    = ERR;
		FO->FAHArray[ix].HomeAgentAddress    = ERR;
		FO->FAHArray[ix].MobileUnitLifeTime  = ERR;
		FO->FAHArray[ix].IsStillRegistered   = ERR;
		FO->FAHArray[ix].TimeIWasRegistered  = ERR;
	}
	return (FO);
}

ForeignAgentsRegistered
	findFAHR(FARObj* FAHR,int CoA)
{
		FARObj* this = ((FARObj*)FAHR);
		assert(CoA < MAX_CARE_OF_ADDRESS);
		return (this->FAHArray[CoA]);
}

/* will simply add a MN to the routers FA table */
int setFAHR(FARObj* FAHR,PACKET_size_t PKT)
{
		int CoA = REQT_checkCareOfAddress          (PKT);
		int HMA = REQT_checkHomeAddress            (PKT);
		int HAA = REQT_checkHomeAgentAddress 	   (PKT);
		int MUL = REQT_checkLifeTimeOfRegistration (PKT);
		time_t right_now;

		FARObj* this = ((FARObj*)FAHR);
		
		printf("CoA %d\n",CoA);
		printf("HMA %d\n",HMA);
		printf("HAA %d\n",HAA);
		printf("MUL %d\n",MUL);
		
		if( ((this->FAHArray[CoA].HomeAddress == ERR) && 
			(this->FAHArray[CoA].HomeAgentAddress == ERR)) 
			||
			((HMA == this->FAHArray[CoA].HomeAddress) && 
		    (HAA == this->FAHArray[CoA].HomeAgentAddress))
		    ||
			this->FAHArray[CoA].IsFieldBeingUsed == FALSE)
		{
			this->FAHArray[CoA].IsFieldBeingUsed    = TRUE;
			this->FAHArray[CoA].CareOfAddress 	    = CoA;
			this->FAHArray[CoA].HomeAddress 	    = HMA;
			this->FAHArray[CoA].HomeAgentAddress    = HAA;
			this->FAHArray[CoA].MobileUnitLifeTime  = MUL;
			this->FAHArray[CoA].IsStillRegistered   = TRUE;
			this->FAHArray[CoA].TimeIWasRegistered  = time(&right_now);
		}
		else
		{
				printf("HA %d | HmA: %d\n",
						this->FAHArray[CoA].HomeAgentAddress,
						this->FAHArray[CoA].HomeAddress);
	    		return ERR;
		}
		return (0);
}

/*
 * if the registration expires we will simply
 * just set the AmIRegistered to false and also
 * Set the IsFieldBeingUsed to false also
 */
int showFATable(FARObj* FA)
{
		FARObj* this = ((FARObj*)FA);
		int ix = 0;
		printf ("TABLE BELOW SHOWS REGISTERED MOBILE UNITS AT THIS FOREIGN AGENTS\n");
		for(ix = 0;ix < this->MAX_ARRAY_SIZE ;ix++)
		{
				if(this->FAHArray[ix].IsFieldBeingUsed == TRUE)
				{
						int CARE_OF_ADDR =  this->FAHArray[ix].CareOfAddress;
						int HOME_ADDR    =  this->FAHArray[ix].HomeAddress;
						int HOME_AGNT    =  this->FAHArray[ix].HomeAgentAddress;
						int LIFETIME     =  this->FAHArray[ix].MobileUnitLifeTime;
						int ISREGED      =  this->FAHArray[ix].IsStillRegistered;
						printf("| CoA: %d | HMA: %d | HAG: %d | LFTL %d | ISREGED : %s\n",
										CARE_OF_ADDR,HOME_ADDR,HOME_AGNT,LIFETIME,(ISREGED)?("TRUE"):("FALSE"));
				}
		}
		return 0;
}

/*----------------------------------------------------------------*/

HARobj* new_HARobj()
{
	int ix = 0;
	HARobj* HA = (HARobj*)malloc(sizeof(HARobj));
	HA->MAX_ARRAY_SIZE = MAX_HOME_ADDRESS;
	for (ix = 0;ix < MAX_HOME_ADDRESS;ix++)
	{
		HA->HARArray[ix].IsFieldBeingUsed = FALSE;
	}
	return (HA);
}

/* will find a row in the HA table */
HomeAgentsRegistered
	findHAR(HARobj* me,int HomeAddress)
{
	HARobj* this = ((HARobj*)me);
	assert(HomeAddress < MAX_HOME_ADDRESS);
	return (this->HARArray[HomeAddress]);
}

/* will simply add a MN to the HA table */
int setHAR(HARobj* me, PACKET_size_t PKT)
{
	int HMA = REQT_checkHomeAddress            (PKT);	
	int CoA = REQT_checkCareOfAddress          (PKT);
	/* 
		the source address is the foreign address of where the
		Mobile Unit Currently IS ....
	 */
	int FNA = REQT_checkSourceAddressOfPacket  (PKT);  
	int HNA = REQT_checkHomeAgentAddress       (PKT);
	time_t right_now;
	
	HARobj* this = ((HARobj*)me);
	
	printf("HomeAddress   : %d\n",HMA);
	printf("CareOfAddress : %d\n",CoA);
	printf("FrgnNwkAgent  : %d\n",FNA);
	printf("HomeNwkAgent  : %d\n",HNA);
	
	this->HARArray[HMA].IsFieldBeingUsed    = TRUE;
	this->HARArray[HMA].HomeAddress   		= HMA;
	this->HARArray[HMA].CareOfAddress 		= CoA;
	this->HARArray[HMA].ForeignAgentAddress = FNA;
	this->HARArray[HMA].HomeNetworkAddress  = HNA;
	this->HARArray[HMA].TimeIWasRegistered  = time(&right_now);
	this->HARArray[HMA].IsStillRegistered   = TRUE;

	return 0;
}

int showHATable(HARobj* HA)
{
	HARobj* this = ((HARobj*)HA);
	int ix = 0;
	printf ("TABLE BELOW SHOWS REGISTERED MOBILE UNITS AT THIS HOME AGENT \n");
	for(ix =0;ix < this->MAX_ARRAY_SIZE ;ix++)
	{
			if(this->HARArray[ix].IsFieldBeingUsed == TRUE)
			{
					int HOME_ADDR         =  this->HARArray[ix].HomeAddress;
					int CARE_OF_ADDR      =  this->HARArray[ix].CareOfAddress;
					int FORN_AGNT_ADDR    =  this->HARArray[ix].ForeignAgentAddress;
					int HOME_NWK_ADDR     =  this->HARArray[ix].HomeNetworkAddress;
					printf("| HMA: %d | CoA: %d | FAA: %d | HNWADDR %d |\n",
									HOME_ADDR,CARE_OF_ADDR,FORN_AGNT_ADDR,HOME_NWK_ADDR);
			}
	}
	return 0;
}

/*----------------------------------------------------------------*/
HAddrToCAddrOBJ* new_HAddrToCAddrOBJ()
{
	HAddrToCAddrOBJ* H = (HAddrToCAddrOBJ*)malloc(sizeof(HAddrToCAddrOBJ));

	H->MAX_ARRAY_SIZE = MAX_MOBILE_UNITS_IN_NETWORK;
	H->findHomeAddressBYCOA   = findHomeAddressBYCOA;
	H->findHomeAddressBYHA    = findHomeAddressBYHA;
	H->findCareOfAddressBYHA  = findCareOfAddressBYHA;
	H->findCareOfAddressBYCOA = findCareOfAddressBYCOA;
	H->getHAddrToCAddrStruct  = getHAddrToCAddrStruct;
	H->setHomeAddress         = setHomeAddress;
	H->setCareOfAddress       = setCareOfAddress;
	H->setHAddrToCAddrSocket  = setHAddrToCAddrSocket;
	H->clearAllvalues         = clearAllvalues;

 	/* Must initialise the contents of the Array to AmIStillValid to false */
	
	return (H);
}


int findHomeAddressBYCOA
	(
		void* me,
		int careOfAddress
	)
{
	HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	int ix = 0;
	for (ix = 0; ix < this->MAX_ARRAY_SIZE ;ix++)
	{
		if(this->HACAarray[ix].uint_CareOfAddr == careOfAddress)	
			return (getHAddrToCAddrStruct(this,ix).uint_HomeAddr);
	}
	return (ERR);
}

int findHomeAddressBYHA
	(
		void* me,
		int HomeAddress
	)
{
	HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	int ix = 0;

	for (ix = 0; ix < this->MAX_ARRAY_SIZE; ix++)
	{
		if(this->HACAarray[ix].uint_HomeAddr == HomeAddress)
			return (getHAddrToCAddrStruct(this,ix).uint_HomeAddr);
	}
	return (ERR);
}

int findCareOfAddressBYHA
	(
		void* me,
		int HomeAddress
	)
{
	HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	int ix = 0;

	for (ix = 0; ix < this->MAX_ARRAY_SIZE ;ix++)
	{
		if (this->HACAarray[ix].uint_HomeAddr == HomeAddress)
			return (getHAddrToCAddrStruct(this,ix).uint_CareOfAddr);
	}
	return 0;
}

int findCareOfAddressBYCOA
	(
		void* me,
		int CareOfAddress
	)
{
		HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
		int ix = 0;

		for(ix = 0;ix < this->MAX_ARRAY_SIZE ;ix++)
		{
			if (this->HACAarray[ix].uint_CareOfAddr == CareOfAddress)
				return(getHAddrToCAddrStruct(this,ix).uint_CareOfAddr);
		}
		return 0;
}

HAddrToCAddr getHAddrToCAddrStruct
	(
		void* me,
		int index
	)
{
	HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);

	assert(index < this->MAX_ARRAY_SIZE);
	return (this->HACAarray[index]);
}

int setHomeAddress
	(
		void* me,
		int HomeAddress
	)
{
	//HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	return 0;
}

int setCareOfAddress
	(
		void* me,
		int CareOfAddress
	)
{
	//HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	return 0;
}

int setHAddrToCAddrSocket
	(
		void* me,
		void* sockT
	)
{
	//HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	return 0;
}

int clearAllvalues
	(
		void* me
	)
{
	//HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	return 0;
}

int initHAddrToCAddrOBJ
	(
		void* me
	)
{
	//HAddrToCAddrOBJ* this = ((HAddrToCAddrOBJ*)me);
	return 0;
}
/* ---------HAddrToCAddrOBJ DONE !---------------------- */

CareOfAddressTableOBJ* new_CareOfAddressTableOBJ()
{
	CareOfAddressTableOBJ* C = (CareOfAddressTableOBJ*)malloc(sizeof(CareOfAddressTableOBJ));

	C->MAX_ARRAY_SIZE = MAX_CARE_OF_ADDRESS;
	C->getCOAAddress = getCOAAddress;
	C->setCOAUsedBit = setCOAUsedBit;
	C->getCOAUsedBit = getCOAUsedBit;
	return (C);
}

/*
  if a negative value is returned it means
  that there are no more free COAAddresses
  and we deny the registration of the Mobile UNIT
 */
int getCOAAddress
	(
		void* me,
		int  ix
	)
{
	CareOfAddressTableOBJ* this = ((CareOfAddressTableOBJ*)me);
	assert (ix < MAX_CARE_OF_ADDRESS);
	return (this->COATableArray[ix].COA);
}

int setCOAUsedBit
	(
	    void* me,
		int   ix,
		int   setVal
	)
{
	CareOfAddressTableOBJ* this = ((CareOfAddressTableOBJ*)me);
	this->COATableArray[ix].COA_taken = setVal;
	return 0;
}

int getCOAUsedBit
	(
		void* me,
		int   ix
	)
{
	CareOfAddressTableOBJ* this = ((CareOfAddressTableOBJ*)me);
	assert (ix < MAX_CARE_OF_ADDRESS);
	return (this->COATableArray[ix].COA_taken);	
}

int initCareOfAddressTableOBJ
	(
		void* me
	)
{
	CareOfAddressTableOBJ* this = ((CareOfAddressTableOBJ*)me);
	int ix = 0;

	for(ix = 0;ix < MAX_CARE_OF_ADDRESS;ix++)
	{
			this->COATableArray[ix].COA = ix+1;
			this->COATableArray[ix].COA_taken = FALSE;
	}
	return 0;
}

/* ----------- CareOfAddressTableOBJ DONE ! -------------- */

RoutingTableOBJ* new_RoutingTableOBJ()
{
	RoutingTableOBJ* R = (RoutingTableOBJ*)malloc(sizeof(RoutingTableOBJ));

	R->MAX_ARRAY_SIZE     = MAX_ROUTERS_IN_NETWORK;
	R->setDestination 	  = setDestination;
	R->setDistance 		  = setDistance;
	R->getDistance 		  = getDistance;
	R->setNextHopRoute    = setNextHopRoute;
	R->getNextHopRoute    = getNextHopRoute;
	R->setNextRouteSocket = setNextRouteSocket;
	R->getNextRouteSocket = getNextRouteSocket;
	R->setNextRouterHost  = setNextRouterHost;
	R->getNextRouterHost  = getNextRouterHost;
	R->showRoutingTable   = showRoutingTable;

	initRoutingTableOBJ(R);
	
	return (R);
}

int showRoutingTable(void* me)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	int ix = 0;
	char digit[20];
	printf("\n");
	for(ix = 0;ix < this->MAX_ARRAY_SIZE;ix++)
	{
	    if (this->RTArray[ix].HasFieldBeenTaken == TRUE)
		{
			sprintf(digit,"%d",this->RTArray[ix].intDistance);
		printf
		(
			"D Nwk %s | Next Hop Router %s | Distance %s | Socket %d | RouterHost %s \n",
			this->RTArray[ix].chDestination,
			this->RTArray[ix].chRoute,
			(this->RTArray[ix].intDistance >= 12)?("INF"):(digit),
			this->RTArray[ix].NextRouteSocket,
			this->RTArray[ix].NextRouterHost
		);
		}
	}
	return 0;
}

int setNextRouterHost
	(
		void* me,
		int network,
		char* hostOfRouter
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);

	this->RTArray[network].HasFieldBeenTaken = TRUE;
	strcpy(this->RTArray[network].NextRouterHost,hostOfRouter);
	return 0;
}

char* getNextRouterHost
	(
		void* me,
		int network
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);

	assert(network < this->MAX_ARRAY_SIZE);
	return this->RTArray[network].NextRouterHost;
}


RoutingTable findNetworkDestination
	(
		void* me,
		int network_pkey
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);	
	
	assert(network_pkey < this->MAX_ARRAY_SIZE);
	return (this->RTArray[network_pkey]);
}

/* this is the primary key of the table */
int setDestination
	(
		void* me,
		int network
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	//assert(this->RTArray[network].HasFieldBeenTaken == FALSE);

	/* we use this for send through the network */
	this->RTArray[network].intDestination = network;

	/* we use this for displaying print etc */
	sprintf(this->RTArray[network].chDestination,"N%d",network);
	return 0;
}

int setDistance
	(
		void* me,
		int network,
		int distance
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);

	this->RTArray[network].HasFieldBeenTaken = TRUE;
	this->RTArray[network].intDistance = distance;
	return 0;
}


int getDistance
	(
		void* me,
		int network
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);
	return (this->RTArray[network].intDistance);
}

int setNextHopRoute
	(
		void* me,
		int network,
		int nextRouter
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);

	this->RTArray[network].HasFieldBeenTaken = TRUE;
	this->RTArray[network].intRoute = nextRouter;
	sprintf(this->RTArray[network].chRoute,"R%d",nextRouter);
    
	/* 
	    do not hardcode this 7000, this must be made a global parameter 
		maybe just make it commandline parameter !
	 */
	this->RTArray[network].NextRouteSocket = 7000 + nextRouter;
	return 0;
}

int getNextHopRoute
	(
		void* me,
		int network
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);
	return(this->RTArray[network].intRoute);
}

int setNextRouteSocket
	(
		void* me,
		int network,
		int socketRange
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);
	/* 
	    this will be the socket that the application will connect to to 
		talk to the routers

		Remember change it so that we dont hardcode the 7000
	*/
	this->RTArray[network].HasFieldBeenTaken = TRUE;
	this->RTArray[network].NextRouteSocket = socketRange + network;
	return 0;
}

int getNextRouteSocket
	(
		void* me,
		int network
	)
{
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);
	assert(network < this->MAX_ARRAY_SIZE);
	return(this->RTArray[network].NextRouteSocket);
}

int initRoutingTableOBJ
	(
		void* me
	)
{
	int ix = 0;
	RoutingTableOBJ* this = ((RoutingTableOBJ*)me);

	for(ix = 0;ix < this->MAX_ARRAY_SIZE;ix++)
	{
		this->RTArray[ix].HasFieldBeenTaken     = FALSE;
		this->RTArray[ix].intDestination        = ERR;
		this->RTArray[ix].intRoute              = ERR;
		this->RTArray[ix].intDistance           = ERR;
		this->RTArray[ix].NextRouteSocket       = ERR;

		strcpy(this->RTArray[ix].NextRouterHost,"127.0.0.1");
	}
	return 0;
}
