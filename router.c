/*-----------------------------------------------------------------------*
 * This file represents the main router process that will reroute the
 * icmp, ip, rip, reply and request packets that are sent
 * 
 * At periodic times this router will send agent advertisment messages
 * depending on what type of agent the router is. If the router is a FA
 * the router will send periodic FA advertisments, if the router is a 
 * HA the router will send periodic HA advertisments.
 *
 *-----------------------------------------------------------------------*
 * CODED BY: SIDNEI BUDIMAN 2003   (C)                                   *
 *-----------------------------------------------------------------------*
 */

#include "router.h"
#include "router_switch_fabric.h"

/*
 * This mutual exclusion is used to block the routing tables
 *  when a packet is being sent and read at the same time
 */
static pthread_mutex_t router_mtex;


/*
 * This function denotes the main creation function for the router
 * All detials that represent this router will be stored in this 
 * structure. It will kepp all the details of this router such as 
 * port numbers, address etc.
 */
Router* ROUTER_newROUTER(char* configuration)
{
	/*
	 * creates the router struct
	 */
	Router* Rtr = (Router*)malloc (sizeof (Router));
	
	initConfigData();
	ScanConfigFileFirst(configuration);
	
 	/*
	 * The intRouterID represents the routers id or routers network
	 */	
	Rtr->intRouterID 	   = ThisRouterConfig.intRouterID;
	/*
	 * This is the input port or the input chanel of the router
	 */
	Rtr->intRouterRecvPort 	   = ThisRouterConfig.intRouterSocketRange 
							+ Rtr->intRouterID;
	/*
	 * This defines whether or not the routing table should be
	 * sent to neighours
	 */
	Rtr->sendRoutingTable      = TRUE;
	/*
	 * both variables denote if the router is a home agent or
	 * if the router is a foreign agent
	 */
	Rtr->BOOLIsRouterHomeAgent = ThisRouterConfig.BOOLIsRouterHomeAgent;
	Rtr->BOOLIsRouterFrgnAgent = ThisRouterConfig.BOOLIsRouterFrgnAgent;
	
	/* char representation of this router */
	sprintf(Rtr->charRouterID,"R%d",Rtr->intRouterID);

	/* simple statistics on the number of packets processed in total */
	Rtr->intRouterPacketsProcessed = 0;
    
	/* lets keep this initialising within this local bracket*/
	{
		int ix = 0;
		for(ix = 0; ix < MAX_ROUTERS_IN_NETWORK; ix++)
		{
				Rtr->FAILED_CNT[ix].failed_count = 0;
		}
	}

	/* a string to represent the host the router is running on !*/
	strcpy (Rtr->charHostRouter,ThisRouterConfig.intHostRouterIsRunning);

	return (Rtr);
}

/*
 * This function is where we setup the details of the router.
 * Here we will create the input ports for the router to listen on.
 * We will also start the agent advertisments here as threads  
 */
int ROUTER_init(char* this_rtr_cfgs)
{
    	/* set up required data structures and globals here */
	ROUTERTable = new_RoutingTableOBJ();
	CONNECTChan = new_RCChan();
	COATable    = new_CareOfAddressTableOBJ();
	ThisRouter  = ROUTER_newROUTER(this_rtr_cfgs);
	HAMobUnits  = new_HARobj();
	FAMobUnits  = new_FARObj();
	
	/* make calls to relavant functions here  to initialise the router */
	initCareOfAddressTableOBJ(COATable);
	InitRouterChannel(CONNECTChan, ROUTER_Callback);
#ifdef DEBUG_ME	
	printf(
			"Host: %s | Port: %d\n",
			ThisRouter->charHostRouter,
			ThisRouter->intRouterRecvPort
		  );
#endif	
	InitServerSocket (
				/* the routers connection channel */
				CONNECTChan,
				/* the host the router will be running on */
				ThisRouter->charHostRouter,	
			   	/* the port the router will be listening on */
				ThisRouter->intRouterRecvPort
			 );
	/*
	 * This function will inititalise the routing table from the
	 * configuration file. It will initialise the neighbours of
	 * this router.
	 */
	ROUTER_InitRoutingTableFromConfigFile();
	
	/*
	 * This function will now start the child threads that will
	 * send the routing tables to the neighbours
	 */
    	ROUTER_sendRoutingTablePeriodic();
	/*
	 * This function will now start the thread that will send the
	 * icmp packets to the neighbours periodically
	 */
	ROUTER_advertiseICMPPeriodic(); 
	/*
	 * This is a timer thread that will check the FA table to see
	 * that if there are entries whether or not those entries are
	 * still within the valid times
	 */
	ROUTER_CheckIsMUnitStillValidAtFAStart();
	/*
	 * This function will eventually bring up the socket
	 * interface that this router will listen on
	 */
	ROUTER_bringUpSocketIntf();
	return 0;
}

/*
 * This function will determine the connectivity status of the routers when
 * they come up. When a router is started it will send a packet to its 
 * neighbours. If the sending of the pkt was successful that neighbour router is
 * set to direct hop otherwise that neighbour is failed in the routing table
 */
int  ROUTER_DetermineConnectivity()
{
	int ix = 0;
	for (ix = 0;ix < ROUTERTable->MAX_ARRAY_SIZE;ix++)
	{
		 BOOLEAN HasBeenTaken = ROUTERTable->RTArray[ix].HasFieldBeenTaken;
		 int     NextHopDist  = ROUTERTable->getDistance(ROUTERTable,ix);
		 if(HasBeenTaken == TRUE && NextHopDist == DIRECT)
		 {
			if (ROUTER_DispatchPacketWait(0,ix) == ERR)
			{
#ifdef DEBUG_ME			
	printf("Setting Network N%d to Distance %d\n",ix,RIP_MAX_HOP);
#endif
				ROUTERTable->setDistance
				(
					/* routing table */
					ROUTERTable,
					/* network to dispatch to */
					ix,
					/* the maximum hop count*/
					RIP_MAX_HOP
				);
			}
	    }
	}
	return 0;
}

/*
 * This router will send the routing tables to the neighbouring
 * routers periodically. This function will start a seperate thread
 * that will start the sending of the routing table
 */
int   ROUTER_sendRoutingTablePeriodic()
{
	/*
	 * determine connectivity first, if the neighbouring routers
	 * are connected and running, then flag them as direct, otherwise
	 * flag them as failed
	 */
	ROUTER_DetermineConnectivity();
	if (pthread_create
			(
				&ThisRouter->tableDispatchThreadID,
				NULL,ROUTER_sendRoutingTableWrapper,NULL
			)
	   )
	{
#ifdef DEBUG_ME
		printf("ERROR Occured in Routing table Dispatch \n");
#endif
	}
	return 0;
}

/* 
 * simple thread wrapper so that the function we want to thread
 * has the same prototype as that expected from pthread_create
 */
void* ROUTER_sendRoutingTableWrapper(void* param)
{
	while (TRUE)
	{
		if (ThisRouter->sendRoutingTable == TRUE)
		{
			/*
			 * This is the actual function that will send
			 * the routing table to the neighouring routers
			 */
			ROUTER_sendRoutingTableToNeigbour();
			/*
			 * we sleep every 10 seconds betwen sending of
			 * the routing tables
			 */
			sleep (10);
		}
		else
		{
			/*
			 * If the sendRoutingTable is flagged as false
			 * then we sleep for longer because this means
			 * that the router has converged
			 */
			sleep (60);
			/*
			 * once the router has slept flag it back to 
			 * send again incase the network has changed
			 * i know we dont need to but it doesnt matter
			 */
			ThisRouter->sendRoutingTable = TRUE;
		}

	}
	return 0;
}

/*
 * This function is the function that will go though the routing table
 * and attempt to send the routing table to all its neighbours
 */
int ROUTER_sendRoutingTableToNeigbour()
{
	int ix = 0;
	/*
	 * we use blocks here because we read from this table aswell as
	 * write to this table from seperate threads so we must ensure that
	 * a whole read or write is complete before another thread attempts
	 * to modify the routing table
	 */	
	pthread_mutex_lock(&router_mtex);
	for(ix = 0;ix < ROUTERTable->MAX_ARRAY_SIZE; ix++)
	{
		BOOLEAN HasBeenTaken = ROUTERTable->RTArray[ix].HasFieldBeenTaken;
		int     NextHopDist  = ROUTERTable->getDistance(ROUTERTable,ix);
		if((HasBeenTaken == TRUE && NextHopDist == DIRECT))
		{
#ifdef DEBUG_ME
			printf("SENDING TABLE BELOW TO NW%d \n",ix);
#endif
			ROUTERTable->showRoutingTable(ROUTERTable);
			if(ROUTER_sendRoutingTableTo(ix) == ERR)
				/*
				 * if we fail to send the table to a router
				 * we have to fail it here
				 */
				ROUTERTable->setDistance
					(
						ROUTERTable,
						ix, /* network to dispatch to */
						RIP_MAX_HOP
					);
			}
		}
		/*
		 * unlock the thread here
 		 */
		pthread_mutex_unlock(&router_mtex);
		return 0;
}

/*
 * This is the function we use to send the contents of the routing table
 * to the neighbours. We send each row of the routing table one at a time
 * to the neighbour. a completion of the loop means that the whole table
 * has been sent to the neighbours
 */
int ROUTER_sendRoutingTableTo(int NetworkID)
{
	PACKET_size_t PKTS_TO_SEND[MAX_ROUTERS_IN_NETWORK];
	int idx;
	BOOLEAN sendingOK = ERR;

	ROUTER_compileRoutingTable(PKTS_TO_SEND);
	/* addToRIP list here  displays the data on the gui */
	addToRIPList (ThisRouter->intRouterID,NetworkID);

	for (idx = 0;idx < MAX_ROUTERS_IN_NETWORK ;idx++)
	{
		/* make sure we have something to send for this */
		if (PKTS_TO_SEND[idx] != 0)
		{
			/* now send the packet */
			if (ROUTER_DispatchPacketWait
				(
				     PKTS_TO_SEND[idx],
					 NetworkID
				) != ERR
			   )
			{
				sendingOK = TRUE;
			}
		}
	}
	return sendingOK;
}

/*
 * This function will get the complete contents of the routing
 * table and turn it into an array of packets representing the
 * routing table which can then be sent to the neighbours
 *
 * we turn each entry of the routing table in 32bit ints which 
 * can be sent to the neighbours via the sockets
 */
int ROUTER_compileRoutingTable(PACKET_size_t* PKTS_TO_SEND)
{
	int           ix  = 0;
	int           MAX = ROUTERTable->MAX_ARRAY_SIZE;

	RoutingTable* RTEntries = ROUTERTable->RTArray;
	for (ix = 0; ix < MAX ;ix++)
	{
			RoutingTable RT = RTEntries[ix];
			if (RT.HasFieldBeenTaken == TRUE) 
			{
				/*
				 * this function will create the rip packets
				 * from the contents of the routing table
				 */
				PACKET_size_t PKT_SEND = ROUTER_makeRipPackets(RT);
				/*
				 * then it will store it in this array 
				 */
				PKTS_TO_SEND[ix] = PKT_SEND;
			}
			else
			{
			/* make 0 the empty packet we will never send a 0 packet */
			/* hence if we see zero its an error message */
				PKTS_TO_SEND[ix] = 0;
			}
	}
	return (0);
}

/*
 * This function will make the rip packets from the entries
 * inside the routing table.
 * Will convert it to a 32bit unsigned number
 * which will then be sent through the socket

 * NOTE: Each row in the table correspongs to one 32bit number
 */

PACKET_size_t ROUTER_makeRipPackets 
           (
	     RoutingTable RT
	   )
{
		PACKET_size_t PKT_TO_SEND = 0; 

		/* first get the data from the routing table */

		int RTR_SRC = ThisRouter->intRouterID; /* this router address */
		int NWK_ID  = RT.intDestination      ; /* network id in the route */
		int NWK_DST = RT.intDistance         ; /* distance to the above network */
		
		/* 
		 *	now create the packet to send to the other router 
		 * 	starting from here is where we create the rip pkts
		 */

		/* insert the packet type here */
		PKT_TO_SEND = MIPPROTO_putPacketType (PKT_TO_SEND,RIP_PKT_TYPE);
		/*
		 * insert the source address of the pkt here, it will be
		 * this routers ip address or in this case router id
		 */
		PKT_TO_SEND = RIP_putSrcAddrROUTER   (PKT_TO_SEND,RTR_SRC);
		/* insert the network we want to advertise here */
		PKT_TO_SEND = RIP_putNetworkID       (PKT_TO_SEND,NWK_ID); 
		/* insert the distance to that network here  */
	    	PKT_TO_SEND = RIP_putDistanceToNwk   (PKT_TO_SEND,NWK_DST);	

#ifdef DEBUG_ME		
		MIPPROTO_debugPacket(PKT_TO_SEND);
		printf(" PKT_TO_SEND AFTER MADE [ %u ] \n",PKT_TO_SEND);
#endif
		/*
		 * once complete the packet is ready to be sent back to the
		 * calling function so it can be stored in the array 
		 */
		return (PKT_TO_SEND);
}


/*
 * This function will update the given routing table. the parameters to this
 * function is the packet and the routing table in which to update. this func
 * needs to be syncronised because we may read from this table as well as 
 * write to it so if it isnt there could be problems
 */ 
int ROUTER_UpdateMyRoutingTable
	(
		PACKET_size_t PKT,
		RoutingTableOBJ* RTObj
	)
{
	int NET_ID = 0;
	pthread_mutex_lock(&router_mtex);
	NET_ID = RIP_checkNetworkID(PKT);
	
	/*
	 * This part of the code checks to see if the routing table was successfuly
	 * updated. There are many reasons why the table may not get updated and
	 * many of them are valid reasons. If the entry does not need to be added
	 * to the Routing table the failed UpdateCounter will not be updated. if
	 * the updating did fail then this counter will be updated. If the counter
	 * reaches a certain number for every entry in the table the frequancy of
	 * rip messages is slowed down to once per-minute. This represents the
	 * quicent state
	 */
	if (RTALGO_updateTable(PKT,ROUTERTable,ThisRouter->intRouterID) == TRUE)
		ROUTER_updateFailedCounter(NET_ID,FALSE); 
	else
		ROUTER_updateFailedCounter(NET_ID,TRUE); 
	
	/*
	 * This function decides of we should keep sending rip updates 
	 * or not. If we find this is false we flag sendRoutingTable to false
	 * to slow down the updates
	 */
	if(ROUTER_KeepSendingTable() == FALSE)
	{
#ifdef DEBUG_ME
		printf("TABLE HAS SETTLED DOWN AND HAS CONVERGED \n");
		printf("NETID is %d and the COUNT IS %d \n",
				NET_ID,ThisRouter->FAILED_CNT[NET_ID].failed_count
					);
#endif
		ThisRouter->sendRoutingTable = FALSE;
	}
	else
	{
#ifdef DEBUG_ME
		printf("TABLE HAS NOT SETTLED DOWN AND HAS NOT CONVERGED \n");
		printf("NETID is %d and the COUNT IS %d \n",
				NET_ID,ThisRouter->FAILED_CNT[NET_ID].failed_count
					);
#endif
		/*
		 * otherwise we keep it to true
		 * and keep sending once every 10 seconds
		 */	
		ThisRouter->sendRoutingTable = TRUE;
	}

	pthread_mutex_unlock(&router_mtex);
	return 0;
}

/*
 * This function will update the failed counter for that particualr
 * row in the routing table. If this counter gets bigger then the
 * MAX failed for all rows in the routing table we slow down the 
 * dispatching of the routing table to neighbours. To one per-minute
 */
int	ROUTER_updateFailedCounter(int network,BOOLEAN failed)
{
	assert(network < MAX_ROUTERS_IN_NETWORK);
	if (failed == TRUE)
	{
	      ThisRouter->FAILED_CNT[network].failed_count++;
	}
	else if  (failed == FALSE)
	{
	      /* reset the counter back if we get a valid update */
	      ThisRouter->FAILED_CNT[network].failed_count = 0;
	}
	return 0;
}

/*
 * function decides if we should keep sending the routing table or not
 * This is the function that counts the FAILED_LIMIT
 */
BOOLEAN ROUTER_KeepSendingTable()
{
	int ix =  0;
	for (ix = 0;ix < MAX_ROUTERS_IN_NETWORK; ix++ )
	{
		RoutingTable RT = ROUTERTable->RTArray[ix];	
		/*
		 * Dont Let it come from direct 
		 * we only check the counters for the entries in the table
		 * for the entries which are not directly connected to the 
		 * router. 
		 */
		if (RT.HasFieldBeenTaken == TRUE && RT.intDistance != DIRECT)

		{
			/*
			 * WE have found an entry in the routing table which is
			 * less then 3 times failed so we dont stop the
			 * update of the tables every 10 seconds
			 */
#ifdef DEBUG_ME
	printf("FAILED NETID %d cnt %d \n",ix,ThisRouter->FAILED_CNT[ix].failed_count);
#endif
	/* if each of the table entries have failed to update FAILED_LIMIT  times then
	 * we are alowed to say dont update the table anymore
	 */
			if(ThisRouter->FAILED_CNT[ix].failed_count < FAILED_LIMIT)
			{
				return TRUE;
			}
		}
	}
	/*
	 * if we reach here what it means is that the routing table
	 * has been fully converged and we dont need to send anymore packets
	 * because we dont have anymore updates, slow down the sending of the
	 * routing table to the neighbours, saves network traffic in real life
	 * situation  (quicent state)
	 * */
	return FALSE;
}

/*
 *  This is the location where we will set the initial routing table from 
 *  the configuration file.
 *  We will read the configuration file and add it into the routing table
 *  structure.
 */
int ROUTER_InitRoutingTableFromConfigFile(char* configuration)
{
    	/* first get the data from the configuration file */
	/* now do the work of adding this data into the table */
	{
		int ix = 0;
		for(ix = 0;ix < MAX_NEIGHBOURS_FILEIO ; ix++)
		{
		    FileData CurrFD = CONFIG_DATA[ix];
			if(CurrFD.validData == TRUE)
			{
				ROUTERTable->setDestination(ROUTERTable,CurrFD.intRouterNwk);
	/* --------------------------------------------------------------------
		if it is a neighboring router we will set the distance of that
		router to DIRECT which is hash defined as zero. This will be
		up dated to the correct value once the routers start to comm-
		unicate from each other.
	   -------------------------------------------------------------------*/
				ROUTERTable->setDistance
				    (
					    ROUTERTable,
				 	    CurrFD.intRouterNwk,
				 	    DIRECT
				    );
	
	/* 
		This section will add the ip address of the host in which
		this router is running on. In this case of the assignement
		this will always be defined as 127.0.0.1 for the local host
	 */
				ROUTERTable->setNextRouterHost
				    (
					    ROUTERTable,
					    CurrFD.intRouterNwk,
					    CurrFD.chRouterRunningHost
				    );
		/*
			This section will set the router for the next hop
			possible values for this field are as follows
			N1, N2, N3 , N4, N5
		 */
				ROUTERTable->setNextHopRoute
				    (
					    ROUTERTable,
				 	    CurrFD.intRouterNwk,
				 	    CurrFD.intRouterAddress
				    );
			}
		}
	}
	return 0;
}

/*
 *	This is the function that handles the sending of the packets to 
 *	other destinations or routers. This function accepts to parameters
 *	they are the packet that we wish to dispatch and the network which
 *	to dispatch this packet to.
 *
 *	HOW IT DOES IT:
 *		The way ths function works is it will lookup the network
 *		which to dispatch the pacekt to by the parameter value
 *		whichNetworkToDispatchTo and look up the routing table
 *		for the corresponding port which to dispatch ths packet
 *		to.
 */
int ROUTER_DispatchPacketWait
	(
		PACKET_size_t PKT,
		int whichNetworkToDispatchTo
	)
{
	/*
	 * From the routing table get all the data we need to be able
	 * to send this though the socket.
	 * we need the following :
	 * 	- PortNumber
	 *      - Host Address that the router process is running on
	 *      - also prepare the packet for transmission
	 */

	 /*
	 	RT denotes the field in the routing table which holds all the 
		information needed to dispatch a packet to that network
	  */
    RoutingTable RT = findNetworkDestination
					(
					/*
					 * from the routing table get the
					 * required field.
					 * whichNetworkToDispatchTo is the
					 * primary key for the routing table
					 */
						ROUTERTable,
						whichNetworkToDispatchTo
					);
	/*
	 * This port defines the port of the next hop router
	 */
	int   PortNumber = RT.NextRouteSocket;
	/*
	 * This variable defines where the router process is
	 * actually running
	 */
	char* HostIpAddr = RT.NextRouterHost;
	char  PKTbuff[20];

	/* 
	 * If this evaluates to FALSE it means the entry is not yet in
	 * the routing table. This should never be the case but if it
	 * ever does get into this state atleast the program will not
	 * crash but will just return an error message
	 */
	if (RT.HasFieldBeenTaken == FALSE)
	{
#ifdef DEBUG_ME
	printf ("Network %d not found in routing table cannot dispatch packet\n"
						,whichNetworkToDispatchTo);
#endif
		return ERR;
	}
	/*
		Now pack the packet we want to send
		IE convert it from an long long type to a 
		char type so it can be sent through the socket
	 */
	MIPPROTO_PackPacket(PKT,PKTbuff);

#ifdef DEBUG_ME
	printf ("HostIpAddr: %s \n",HostIpAddr);
	printf ("PrtNumber : %d \n",PortNumber);
	printf ("Packet    : 0x%x\n",PKT);
#endif 
	
	/*
	 * Now dispatch this packet though the socket
	 * BUT We will sleep before we dispatch the packet so that
	 * we dont send it to fast
	 * ALSO: 
	 * This is so the reciving port does not get qued up so fast
	 * and so the ques are evenly spread out
	 */
	sleep(WAIT_PERIOD_B4_SEND);
	if (DispatchPacket
			(
				DISPATCH_PACKET_WRITER,
				PortNumber,
				HostIpAddr,
				PKTbuff
			) == ERR) 
	{
#ifdef DEBUG_ME
		printf("ERROR WHEN DISPATCHING TO NETWORK [ N%d ] .....\n",
						whichNetworkToDispatchTo);
#endif
		return (ERR);
	}
	return 0;
}

/*
 * function is obselete, we cant send the routing table as a new
 * thread, something happens (sync problem) do not use this function
 */
void* ROUTER_DispatchWrapper( void* thdData )
{
	sleep (((ThreadData*)thdData)->dispatchFromNowTime);
	ROUTER_DispatchPacketWait
			(
				((ThreadData*)thdData)->PKT,
				((ThreadData*)thdData)->whichNetworkToDispatchTo
			);
	return 0;
}

/* 
 * SAME AS ABOVE DO NOT USE THIS FUNCTION
 */
int ROUTER_DispatchPacketDontWait
	(
		PACKET_size_t PKT,
		int whichNetworkToDispatchTo,
		int dispatchFromNowTime
	)
{
	ThreadData TData;
	pthread_t  thrdID;

	TData.PKT = PKT;
	TData.whichNetworkToDispatchTo = whichNetworkToDispatchTo;
	TData.dispatchFromNowTime = dispatchFromNowTime;
	
	if (pthread_create(&thrdID,NULL,ROUTER_DispatchWrapper,((void*)&TData)))
	{
		printf ("Packet %u 0x%x could not be dispatched\n",PKT,PKT);
	}
	return 0;
}

/*
 *	This is where the router will bring up its input interface or
 *	its socket that it will listen on. When this function is called the
 *	router will go into an infinite wait to wait for packets which it 
 *	will then process
 */
int ROUTER_bringUpSocketIntf()
{
	close (CONNECTChan->ServerSocket);
	while (TRUE)
	{
		/*
			function used to wait for events 
		 */
		WaitForEvents
		(
			CONNECTChan,
			WRITE_BACK_TO_SOCK_SERVER
		);
		/* 
			allows us to count the number of packets recieved 
			This was used for debugging but its ok to leave it here
		 */
		((int)(ThisRouter->intRouterPacketsProcessed))++;

		/*
		 * if we need to do somthing after every recived packet
		 * we can accompish it here: at the moment though i have
		 * nothing i need to do here os its empty
		 */
	}
}

/*------- THESE FUNCTIONS HANDLE THE ROUTERS ICMP ADVERTISMENTS ----------*/

/*
	The router has a table of care-of-addresses it can use. This function
	will get the next free care of address and return it. This function is
	used in the making of the icmp advertisment messages

 */
int ROUTER_getFreeCareOfAddress()
{
	int MAX = COATable->MAX_ARRAY_SIZE;
	int ix        = 0;
	int FREE_COA;

	for (ix = 0;ix < MAX;ix++)
	{
#ifdef DEBUG_ME
		printf("ROUTER_getFreeCareOfAddress looping ....\n");
#endif
		/*
		  if the used bit is set to false then we know this is
		  a free care of address and we will return this
		 */
		if (COATable->getCOAUsedBit(COATable,ix) == FALSE)
		{
			FREE_COA = COATable->getCOAAddress(COATable,ix);
			return (FREE_COA);
		}

	}
	return ERR;
}

/*
	This function will simply dispatch a thread which it will use 
	to keep sending icmp advertisment messages
 */
int ROUTER_advertiseICMPPeriodic()
{
	/* 
	 * first we gotta check if this router is a foreign agent or a home agent before we
	 * start dispatching ICMP packets. The reason is if its a home agent we need to
	 * dispatch icmp packets to advertise home packets and if is a foreign agent 
	 * we will need to advertise forign packets. 
  	 *
	 * The type of packet to advertise will be determined within the actual
	 * ROUTER_advertiseAsICMPNetworkWrapper function
	 */ 
	if (
		ThisRouter->BOOLIsRouterFrgnAgent == TRUE ||
		ThisRouter->BOOLIsRouterHomeAgent == TRUE
	   )
	{
		/*
		 	Create a thread to start the advertisments
		 */
	   if (pthread_create
		  (
			&ThisRouter->icmpDispatchThreadID,
			NULL,ROUTER_advertiseAsICMPNetworkWrapper,NULL
		  ))
	   {
 		printf("pthread could not start for ROUTER_advertiseAsFAPeriodic \n");
	   }
	}
	else
		printf("Router R%d is not a FA \n",ThisRouter->intRouterID);
	return 0;
}

/* simple wrapper to that pthread can accept this function */
void* ROUTER_advertiseAsICMPNetworkWrapper(void* param)
{
		while (TRUE)
		{
			ROUTER_advertiseICMPNetwork();
			/*
			  Will broadcast every 5 seconds
			 */
			sleep(5);
		}
		return 0;
}

/*
 *	This packet will make the icmp packets. This function will
 *	first determine of the router is a FA or HA then depending
 *	on that situation it will make the packets accordingly
 */
PACKET_size_t ROUTER_makeIcmpPackets()
{
	PACKET_size_t ICMP_PKT = 0;
	PACKET_size_t ICMP_FLAG_SEGMENT_PKT = 0;
	
	/*
		We set this 2 fields outside the FA or HA comparator because
		all icmp packets need this fields filled so we do it here 
		outside
	 */
	   
	ICMP_PKT = MIPPROTO_putPacketType(ICMP_PKT,ICMP_PKT_TYPE);
	ICMP_PKT = ICMP_putSourceAddressOfPacket(ICMP_PKT, ThisRouter->intRouterID );
	
	/*
		This tests to see if the router is only a foreign agent
	 */
	if ( 
		ThisRouter->BOOLIsRouterFrgnAgent == TRUE &&
		ThisRouter->BOOLIsRouterHomeAgent == FALSE 
	   )
	{
         	    /* set the foreign agent flag */
		    ICMP_FLAG_SEGMENT_PKT = ICMP_setAgentType
				(
					ICMP_FLAG_SEGMENT_PKT,
					ICMP_AGT_FGN /* This says pkt is forign pkt */
				);
		    /* will place the whole Flag segmant into the packet */
		    ICMP_PKT = ICMP_putICMPFlags
				(
					ICMP_PKT,
					ICMP_FLAG_SEGMENT_PKT
				);
		
		    /* for a foreign agent we must set the care of addresses */
		    /* and also the life time that the MU is registered at this FA */

		    {	
			int COA_FREE = 0;

			/*
			  it will equal error if there are no more COA's left
			 */
			if ((COA_FREE = ROUTER_getFreeCareOfAddress()) != ERR)
			{
				ICMP_PKT = ICMP_putAvailCareOfAddress 
					(
						ICMP_PKT,
						COA_FREE,
						ICMP_COA_1
					); 
				
				/*
					This is the lifetime of the MN at the FA
				 */
				ICMP_PKT = ICMP_putLifeTime
					(
						ICMP_PKT,
						LIFE_TIME /* at the moment its 15 */
					);
			}
			else
				printf("CARE OF ADDRESS COULD NOT BE ASSIGNED %d \n",ICMP_COA_1);
		}
	}
	/* if we find out this router is only a HA */
	else if (
			ThisRouter->BOOLIsRouterFrgnAgent == FALSE &&
			ThisRouter->BOOLIsRouterHomeAgent == TRUE
		)
	{
		/* set the home agent flag */
	        ICMP_FLAG_SEGMENT_PKT = ICMP_setAgentType
					(
						ICMP_FLAG_SEGMENT_PKT,
						ICMP_AGT_HME
					);
	        ICMP_PKT = ICMP_putICMPFlags
					(
						ICMP_PKT,
						ICMP_FLAG_SEGMENT_PKT
					);
	}
	/* if its both a foreign && a home agent */
	/* then we need to set all fields in the packet accordingly */
	else if (
			ThisRouter->BOOLIsRouterFrgnAgent == TRUE &&
			ThisRouter->BOOLIsRouterHomeAgent == TRUE
		)
	{
		/* set the home agent flag */
	    	ICMP_FLAG_SEGMENT_PKT = ICMP_setAgentType
				(
					ICMP_FLAG_SEGMENT_PKT,
					ICMP_AGT_HME
				);
	        ICMP_PKT = ICMP_putICMPFlags
				(
					ICMP_PKT,
					ICMP_FLAG_SEGMENT_PKT
				);

			ICMP_PKT = ICMP_putLifeTime
				(
					ICMP_PKT,
					LIFE_TIME /* at the moment its 15 */
				);

			/* set the foreign agent flag */
	        ICMP_FLAG_SEGMENT_PKT = ICMP_setAgentType
				(
					ICMP_FLAG_SEGMENT_PKT,
					ICMP_AGT_FGN
				);
	        ICMP_PKT = ICMP_putICMPFlags
				(
					ICMP_PKT,
					ICMP_FLAG_SEGMENT_PKT
				);
			
		/* for a foreign agent we must set the care of addresses */

	        {	
			int COA_FREE = 0;
			if ((COA_FREE = ROUTER_getFreeCareOfAddress()) != ERR)
				ICMP_PKT = ICMP_putAvailCareOfAddress 
						(
							ICMP_PKT,
							COA_FREE,ICMP_COA_1
						); 
			else
				printf("CARE OF ADDRESS COULD NOT BE ASSIGNED %d \n",ICMP_COA_1);
		}
	}

#ifdef DEBUG_ME
		printf("THE ICMP PKT ABOUT TO BE DISPATCHED LOOKS LIKE ");
		MIPPROTO_debugPacket(ICMP_PKT);
#endif
		return (ICMP_PKT);
}

/*
 * maybe we can do a check, once a communication has been established at this
 * port to the mobile unit we dont loop though this port anymore
 */ 
int ROUTER_advertiseICMPNetwork()
{
	int  ix = 0;
	int  NETWORK_ID = ThisRouter->intRouterID;
	int  NETWORK_TO_BROADCAST_TO = NETWORK_ID * 1000 + PORT_CLEAR; 
	char PKTbuff[20];

	/* gui related */
	char advertP[50]; 
	char ipAddr [20];
	
	PACKET_size_t ICMP_PKT = ROUTER_makeIcmpPackets();
	MIPPROTO_PackPacket(ICMP_PKT,PKTbuff);
	
	/*
	 *	This router will now dispatch the packet to all hosts
	 *	on the network IE a broadcast
	 */
	for (ix= 0;ix < NETWORK_BROADCAST ;ix++ )
	{
		int NETWORK_PORT_DISPATCH_SIM = NETWORK_TO_BROADCAST_TO + ix;
		if (DispatchPacket
				(
				     DISPATCH_PACKET_WRITER,
				     NETWORK_PORT_DISPATCH_SIM ,
				     ThisRouter->charHostRouter,
				     PKTbuff

				) == ERR) 
		{
#ifdef DEBUG_ME
	printf("ERROR WHEN DISPATCHING advertisment TO MUnit to [ Net%d ] on port %d\n",
		NETWORK_ID,NETWORK_TO_BROADCAST_TO - PORT_CLEAR  + ix);
#endif
		}
		else
		{
	printf("SUCCESS WHEN DISPATCHING advertisment TO MUnit ON [Net%d] on port %d\n",
		NETWORK_ID,NETWORK_TO_BROADCAST_TO - PORT_CLEAR + ix);
		}

		/* gui related */	
		convertPortToIpFormat(NETWORK_ID,ix,ipAddr);
		sprintf(advertP,"[%s] Broadcast To Network[%d] Reached: %s\n",
			(ThisRouter->BOOLIsRouterHomeAgent)?("HA"):("FA"),NETWORK_ID,ipAddr);
				
		addToAdvertList(advertP);
		/* gui related */
		sleep(WAIT_PERIOD_B4_SEND);
	}
	return 0;
}

/*
 *	This is the  reply packet back to the MN. When a MN sends a request
 *	packet a router with HA software must eventually send back a reply packet.
 *	This function will help prepare this replay packet according to the 
 *	request packet.
 */
PACKET_size_t ROUTER_PrepareREPLYPKTForFA(PACKET_size_t RQST_PKT)
{
	/* Request packets home address field */
	int HMEAddr = REQT_checkHomeAddress(RQST_PKT);	
	/* Request packets care-of-address field */
	int COAAddr = REQT_checkCareOfAddress(RQST_PKT);
	/* Request packets lifetime registration field */
	int LfeTime = REQT_checkLifeTimeOfRegistration(RQST_PKT);

	/* 
	   This will be the new destination address back to the foreign agent  
	   So whatever the source address of the packet will be will now be the
	   destination address
	 */
	int FGNAgntNwk = REQT_checkSourceAddressOfPacket(RQST_PKT);

	/* 
	   This will be the new source address the current home agent 
	   So whatever the Homeaddress is, it now becomes the source address
	   because this packet now origninates for here
	 */
	int HMEAgntNwk = REQT_checkHomeAgentAddress(RQST_PKT);
	
	/* 
	  prepare the reply back back for resending using data from the rqst pkt 
	  This function simply creates the reply packet to be dispatched back to 
	  the FA
	 */
	PACKET_size_t RPLY_PKT = 0;
	/* add type */
	RPLY_PKT = MIPPROTO_putPacketType(RPLY_PKT,REPLY_PKT_TYPE);
	/* add lifetime*/
	RPLY_PKT = REPLY_putLifeTimeOfRegistration(RPLY_PKT,LfeTime);
	/* add home address*/
	RPLY_PKT = REPLY_putHomeAddress(RPLY_PKT,HMEAddr);
	/* put care-of-address */
	RPLY_PKT = REPLY_putCareOfAddress(RPLY_PKT,COAAddr);
	/* out source address */
	RPLY_PKT = REPLY_putSourceAddressOfPacket(RPLY_PKT,HMEAgntNwk);
	/* put destination address */
	RPLY_PKT = REPLY_putDestinationAddressOfPacket(RPLY_PKT,FGNAgntNwk);
	/* now return the used packet */
	return (RPLY_PKT);	
}

/*
 *	This function will simply check the validity of the MN at the FA.
 *	It will simply check the timmers so see if the MN has overstayed its
 *	welcome. If it has then that MN is flagged as no longer registered
 */
int ROUTER_CheckIsMUnitStillValidAtFAStart()
{
	pthread_t thread_pthread;
	if(ThisRouter->BOOLIsRouterFrgnAgent == FALSE)
	   return 0;

	if (pthread_create
		(
			&thread_pthread,
			NULL,ROUTER_CheckIsMUnitStillValidAtWrapper,NULL
		)
	)
	{
		printf("error in the creation of the valid thread \n");
		return ERR;
	}
	return 0;
}

/*
	Simple wrapper so it satisfies pthread_create
	The check will take place every 5 seconds
 */
void* ROUTER_CheckIsMUnitStillValidAtWrapper
       (
		  void* param
	   )
{
	while(TRUE)
	{
		ROUTER_CheckIsMUnitStillValidFA();
		sleep(5);
	}
	return NULL;
}

/*
 *	Will check to see if the MN is still registered. It will check
 *	the current time with the time the packet was registered if timenow
 *	minus time registered is greater then the lifetime then then the MN is
 *	flagged as false meaning it is no longer registered
 */
int ROUTER_CheckIsMUnitStillValidFA()
{
	int ix = 0;
	for (ix = 0;ix < FAMobUnits->MAX_ARRAY_SIZE;ix++ )
		{
			ForeignAgentsRegistered FR = FAMobUnits->FAHArray[ix];
			if(FR.IsFieldBeingUsed == TRUE)
			{
				time_t time_right_now;
				int    lifeTime = FR.MobileUnitLifeTime;
				time(&time_right_now);
#ifdef DEBUG_ME
	printf("NOW %d  | REG %d \n",time_right_now,FR.TimeIWasRegistered);
#endif
				/*
					OK lets check to see if the time the MN was registered
					minus the time now is greater then the life time. if it
					is then we flag the MN as IsStillRegistered to false 
				 */
				if ((time_right_now - FR.TimeIWasRegistered) >= lifeTime)
				{
					printf("SETTING MU TO NOLONGER REGISTERED \n");
					FAMobUnits->FAHArray[ix].IsStillRegistered = FALSE;
					//FAMobUnits->FAHArray[ix].IsFieldBeingUsed  = FALSE;
					printf("FREEING UP CAREOF ADDRESS %d\n",
						FAMobUnits->FAHArray[ix].CareOfAddress);

					COATable->setCOAUsedBit
						(
							COATable,
							FAMobUnits->FAHArray[ix].CareOfAddress-1,
							FALSE
						);
					ROUTER_ShowTheFATable();
					addToFASpecTable(FAMobUnits,ThisRouter->intRouterID);
				}
			}
		}
	return 0;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 * this is the main function in the router						   *
 * when ever we get a packet this is the                           *
 * function that gets called                                       *
 *                                                                 *
 * This function must be registerd when calling InitRouterChannel  *
 * To see this do a seach on InitRouterChannel to see what i mean  *
 * by registered.                                                  *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 */
int ROUTER_Callback(PACKET_send_t RawPacket)
{
	PACKET_size_t PKT = MIPPROTO_unPackPacket(RawPacket);
#ifdef DEBUG_ME
	printf("PACKETS PROCESSED = %d \n",ThisRouter->intRouterPacketsProcessed);
#endif

	switch (MIPPROTO_getPacketType(PKT))
	{
		/* here we are dealing with REQUEST packet types */
		case REQT_PKT_TYPE  :
		{
			/*
			 *	First check to see if this mobile unit is registering 
			 *	for a home agent We can know this if the HomeAgent
			 *	is the same as this routers ID 
			 *
			 *	if the HA of the MN is the same as this Router
			 *      then its a home agent request. proceed to do this
			 *      work
			 */
			int HOMEAGENT_ID = REQT_checkHomeAgentAddress(PKT);
			if (HOMEAGENT_ID == ThisRouter->intRouterID)
			{
				/*
					Register this MN in the HA table of this
					router
				 */
				ROUTER_RegisterMOBUNITAsHomeHere(PKT);
				addToHASpecTable(HAMobUnits);
				addToMSGList("REQEST TO UPDATE THE HA TABLE WAS MADE",ERR,0);
				ROUTER_ShowTheHATable();
		/* 
		 *	if this evaluates to true it means the packets home agent
		 *	matches this routers address but because the relay flag is
		 *	set it means that it came from a foreign network, not locally
		 *	so we have to confirm registration and send a reply back to
		 *      FA and eventually the Mobile Unit
		 */
				if ((REQT_checkREQTFlags(PKT) & REQT_PKT_IS_A_RELAY) == 
					 REQT_PKT_IS_A_RELAY)
				{
					/*
					  create a reply packet to send back to the
					  FA
					 */
					PKT = ROUTER_PrepareREPLYPKTForFA(PKT);
					ROUTER_DoRouting(PKT,REPLY_PKT_TYPE);
				}
			}
			else
			{
			/*
			 *  bascially we check if the packet came directly from an MU 
			 *  if it did then we register it in the database 
			 *  Then we change the flag to relay because now when this packet
			 *  is resent it will be a request packet with relay set
			 *  
			 *  Also append this router as the new source address
			 *
			 *  NOTE:
			 *  before registering check to see if it already exists
			 *  if it does then check to see if still within timer
			 *  if not then we shall renew the registration
			 */
			
			/*
			 * This part of the code that de-registers is obeselete, we no longer
			 * need to use it because we allow the timmer to automatically de-register
			 * the MN from the FA, we dont need to explicity call de-register but
			 * well just leave the code here anyway incase we need to use it
			 * somewhere else
			 */
			if ((REQT_checkREQTFlags(PKT) | REQT_PKT_DEREGISTER) == 
				  REQT_PKT_DEREGISTER
			   )
			{
#ifdef DEBUG_ME
	printf("A CALL TO DE_REGISTER ON THE ROUTER_HAS BEEN MADE \n");
#endif
				ROUTER_DeRegisterMOBUNITAsFrgnHere(PKT);
				addToFASpecTable(FAMobUnits,ThisRouter->intRouterID);
				ROUTER_ShowTheFATable();
			}
			/*
			 * if  the request pkts HA does not match this routers address
			 * it means that this router is not the HA of the MN so it means
			 * that this router is a FA for the MN and hence needs to register
			 * This MN in the FA table of this router, once this is done
			 * then it needs to resend the request packet
			 */
			else if (
					(REQT_checkREQTFlags(PKT) | REQT_PKT_DIRECT_FROM_MU) ==
					REQT_PKT_DIRECT_FROM_MU
				)
			{
				int CoA_Advertised = 0;
				PACKET_size_t REQST_FLAG_SEGMENT_PKT = 0;
				
				/*
				 *	If we recive this type of request packet we try to register 
				 *	this MN in the Fa table of this router. However if it fails most
				 *	of the time its due to a previous MN already taken the CoA so we
				 *	fail the second MN that tries to get the CoA and make it wait till the
				 *	ICMP adds advertise an untaken CoA
				 */
				if (ROUTER_RegisterMOBUNITAsFrgnHere(PKT) == ERR)
				{
					printf("COULD NOT REGISTER MN AT FA DUE TO CoA Taken Waiting for ANOTHER COA \n");
					addToMSGList("REGISTRATION FAILED COA REQSTED ALREADY TAKEN",ERR,0);
					return ERR;
				}
					
				/* if registration is successful show the FA table */		
				ROUTER_ShowTheFATable();
				addToFASpecTable(FAMobUnits,ThisRouter->intRouterID);
				addToMSGList("UPDATE FA TABLE",ERR,0);

				/* set the pkt as a relay packet now */
				addToMSGList("SETTING REQST PKT TO A RELAY PACKET",ERR,0);
				REQST_FLAG_SEGMENT_PKT = REQST_FLAG_SEGMENT_PKT |
										 REQT_PKT_IS_A_RELAY;

				PKT = REQT_putREQTFlags(PKT,REQST_FLAG_SEGMENT_PKT);
				/* append the source address of this router in the pkt */
				PKT = REQT_putSourceAddressOfPacket
						(
							PKT,
							ThisRouter->intRouterID
						);
				CoA_Advertised = REQT_checkCareOfAddress(PKT);

				/*
				 * variables defined locally because we only need it here
				 * so add curely brackets
				 */
				{	
					char BUFF[40];
					char IP[20];

					convertPortToIpFormat(ThisRouter->intRouterID,CoA_Advertised,IP);
					sprintf (BUFF,"FLAGGING COA [%s] AS NOW TAKEN",IP);
					addToMSGList(BUFF,ERR,0);
				}
#ifdef DEBUG_ME
		printf("SETTING COA [%d] as now USED\n",CoA_Advertised);
#endif
				/* now set this care of address grabbed by the MN to Being used hence TRUE */
				COATable->setCOAUsedBit(COATable,CoA_Advertised-1,TRUE);
			
				/* now route this packet to where it should go */
				ROUTER_DoRouting(PKT,REQT_PKT_TYPE);
			}
			/*
			 *	If the only field defined is the relay flag then we need to just
			 *	send it again and do nothing with it because it means the packet is
			 *	in transit between the FA and HA or HA to FA so do nothing but resend
			 *  it according to the routing table
			 */
			else if (
					  (REQT_checkREQTFlags(PKT) | REQT_PKT_IS_A_RELAY) == 
					   REQT_PKT_IS_A_RELAY
				    )
			{
				/* if its a relay just re-route the pkt and dont do anything else */
				ROUTER_DoRouting(PKT,REQT_PKT_TYPE);
			}
		}	
	}
	break;
	/*
	 *	Lets handle reply packet types here
	 */
	case REPLY_PKT_TYPE :
		{
			int DES_ADDRESS = REPLY_checkDestinationAddressOfPacket(PKT);
			/* 
		     *	If the desiniation ID in the packet is == to the curernt routers ID or 
		     *	address then this means that the packet is at the required FA. What 
		     *	needs to happen now is the packet needs to be sent directly to the
		     *	Mobile Node so we do that here !
			 */
			if (DES_ADDRESS == ThisRouter->intRouterID)
			{
				printf ("SENDING DIRECT TO MU \n");
				addToMSGList("REPLY MSG RECV'd SENDING TO MOBILE UNIT",ERR,0);
				ROUTER_sendPKTToMobileUnit(PKT,REPLY_PKT_TYPE);
			}
			else
			{
				/*
				 * otherwise if they arent equal it simply means that the
				 * packt hasnt reached its required dest and it needs to 
				 * be re-routed according to the routing table
				 */
				printf("RELAYING THIS PACKET TO NEXT DEST \n");
				addToMSGList("REPLY MSG IS BEING RE-ROUTED TO NEXT DEST",ERR,0);
				ROUTER_DoRouting(PKT,REPLY_PKT_TYPE);
			}
		}
		break;
	case RIP_PKT_TYPE :
		{
			/* every rip packet will attempt to update the routing table */
			/* but the routing algorithm will decide if it really needs to be */
			/* updated */
			ROUTER_UpdateMyRoutingTable(PKT,ROUTERTable);
			addToRouterList(ROUTERTable);
#ifdef DEBUG_ME
			ROUTERTable->showRoutingTable(ROUTERTable);
#endif
		}
		break;	

		/* when we get up to the IP packet part lets do a test which says
		 * if the mu is not registered or not the tbe database then 
		 * it cant send anything and we drop the packet here
		 *
		 * */
	case IP_PKT_TYPE :
		{
			/* get the tunneled flag, this will determine if the packet is tunneled or not */
			int TNNL_FLG = IP_checkTunneledPkt(PKT);
			/* get the destiniation network of the packet */
			int DEST_NWK = IP_checkDestNwk(PKT,TNNL_FLG);

			/*
			 *	if its not tunneled we look at the CoA table
			 *	and then re-route the packet to that new address
			 */
			 if (
			 		/* 	
					 *	basically checks if this condition evaluates to true
					 *	it means the pkt is currently at the home agent 
					 */
			 		(DEST_NWK == ThisRouter->intRouterID) && 
					(TNNL_FLG == FALSE)
				)
			 {
			 	/*
				 * Since all IP pkts will go to the home agent first when the HA 
				 * recvs the ip pkt it will firstly attempt to send the pkt to the
				 * HomeAddress of the MN, according to the HA table. If this fails
				 * then it will check the HA table for the CoA of the MN, if the MN
				 * is at another location this will be a valid CoA address and the
				 * pkt will be re-routed to this CoA address
				 */
				if(ROUTER_sendPKTToMobileUnit(PKT,IP_PKT_TYPE) == ERR)
				{
					int HomeAddress = IP_checkDestinationInterface(PKT,TNNL_FLG);
					/*	
					 *	use the HA lookup table to find the corresponding care of address
				     *	for the given home address of the MN
					 */
			 		HomeAgentsRegistered HA = findHAR(HAMobUnits,HomeAddress);
					/*
					  Do we need to check the success of this lookup ?
					 */
			
			/*
			 * Now create the IP pkt with the tunneling enabled and also set the address
			 * such as source address, dest address of this packet to now go to the 
			 * CoA of the MN
			 */
					PKT = IP_putTunneledPkt(PKT,TRUE);
					addToMSGList("HA ROUTER TUNNEL IP PKT",ERR,0);
					/*
					 *	Now lets use the lookup tables data to find out the required
					 *	CoA and Destination network. And create an IP pkt from all this
					 *	Data
					 */
					PKT = IP_putDestNwk (PKT,HA.ForeignAgentAddress,TRUE);
					PKT = IP_putDestinationInterface (PKT,HA.CareOfAddress,TRUE);
					PKT = IP_putSrcNwk(PKT,ThisRouter->intRouterID,TRUE);
					PKT = IP_putSourceInterface(PKT,HomeAddress,TRUE);
#ifdef DEBUG_ME					
					printf("NEW DESTNwk: HA.ForeignAgentAddress %d\n",HA.ForeignAgentAddress);
					printf("NEW DESTInf: HA.CareOfAddress %d\n",HA.CareOfAddress);
					printf("IP PKT After Tunnel is: ");
					MIPPROTO_debugPacket(PKT);
#endif
					/*
					 * 	Now route this packet according to the
					 *	New tunneled Destination Interface and Destination Nwk
					 */
			 		ROUTER_DoRouting(PKT,IP_PKT_TYPE);
				 }
			 }
			 else if (
			 			/*
						 *	If this condidtion evaluates to true it means we are at the
						 *	foreign agent. Because if the Router ID and the ID of the 
						 *	desination is equal AND the pkt has been tunneled then it means
						 *	its on the home strech IE to the FA
						 */
			 			(DEST_NWK == ThisRouter->intRouterID) &&
						(TNNL_FLG == TRUE)
			    	 )
			 {
			 	// <-------------------- IMPORTANT ADD CODE HERE 
				// now check to see the MU is still there and still registerd
				// if it is then send the packet to the MU
				
					printf("IP PKT AT FORIGN UNIT NOW DISPATCH TO MU \n");
					addToMSGList("IP PKT RECV'd ON FA DISPATCHING IP PKT TO MOBILE UNIT",ERR,0);
					if (ROUTER_sendPKTToMobileUnit(PKT,IP_PKT_TYPE) == ERR)
					{
						/* Means dispatching of a pkt has failed to reach the MN for some reason */
						return ERR;
					}
			 }
			 else
			 {
			 	/*
				 * if the Destination network and the RouterID are not the same then
				 * it means the pkt is in transit from either FA to HA or HA to FA so
				 * simply re-route it according to the routing table
				 */
			 	ROUTER_DoRouting(PKT,IP_PKT_TYPE);
			 }
		}
		break;

	default :
			/*
			 *	All un-recoginesed packets are flagged here, if this occurs we need
			 *	to check that the blazes is going on but so far so good do 
			 *	["UNRECOGNISED PKT RECVD"] has ever been printed out
			 */
			printf("UNRECOGNISED PKT RECVD \n");
#ifdef DEBUG_ME
			MIPPROTO_debugPacket(PKT);
			ROUTERTable->showRoutingTable(ROUTERTable);
#endif
		break;
	}
	return 0;
}

/*
 * Simple function that will show the FA table 
 * so we can see the entries
 */
int ROUTER_ShowTheFATable()
{
		showFATable(FAMobUnits);
		return 0;
}

/*
 * Simple function that will show the HA table
 * so we can see the entries of the table
 */
int ROUTER_ShowTheHATable()
{
		showHATable(HAMobUnits);
		return 0;
}

/*
 * This function handles the very important task of sending
 * the different sorts of packets to the Mobile Node. 
 * The parameters are the PKT, the type of PKT 
 *
 * I could have just checked the type from the pkt but this
 * Is easier :-)
 *
 */
int ROUTER_sendPKTToMobileUnit(PACKET_size_t PKT,int PKT_TYPE)
{
	int  FRNNwk 	 = 0;	
	int  CoAMU  	 = 0;
	int  srcNwk 	 = 0;
	int  srcInf 	 = 0;
	int  MUsSendPort = 0;

	char PKTbuff     [20];
	char MSGBUFF     [100];
	char SRCIP       [20];
	
	/*
	 * Will handle the sending of the reply packet back to
	 * the MN to say that it has been registered successfully
	 */
	if(PKT_TYPE == REPLY_PKT_TYPE)
	{
		time_t right_now_t;

		FRNNwk = REPLY_checkDestinationAddressOfPacket(PKT);
		CoAMU  = REPLY_checkCareOfAddress(PKT);

		/* we register this MU as officially registererd just before the reply pkt hits the mobile unit*/
		FAMobUnits->FAHArray[CoAMU].TimeIWasRegistered = time(&right_now_t);
#ifdef DEBUG_ME
		printf("COAMU AT FIRST %d\n",CoAMU);
		printf("COAMu Debug ");
		MIPPROTO_debugPacket(CoAMU);
#endif
		srcNwk = REPLY_checkSourceAddressOfPacket(PKT);
		srcInf = 0; /* source is the network router not a host */
#ifdef DEBUG_ME		
		printf("VALUE OF PKT IS ");
		MIPPROTO_debugPacket(PKT);
#endif
	}

	/*
	 * Will handle the sending of the IP pacekt to the 
	 * Mobile Unit. What we do is get the real source address ie the address
	 * which the IP pkt was originally sent from so when we display it
	 * we know the original source no the HA only
	 */
	else if(PKT_TYPE == IP_PKT_TYPE)
	{
		int IS_TUNNELED = IP_checkTunneledPkt(PKT);
		FRNNwk = IP_checkDestNwk(PKT,IS_TUNNELED);
		CoAMU  = IP_checkDestinationInterface(PKT,IS_TUNNELED);
		/* 
		 *	we want to see the real source not the outside source 
		 *	because then thsi would be just the HA
		 *
		 *  that is why we have FALSE because we want to see the
		 *  state of the src before the tunnel occured
		 */
		srcNwk = IP_checkSrcNwk(PKT,FALSE);
		srcInf = IP_checkSourceInterface(PKT,FALSE);
	}
	
	convertPortToIpFormat(srcNwk,srcInf,SRCIP);
	sprintf(MSGBUFF,"ATTMPT TO SEND AN %s PKT TO THE MNODE, SRC = %s",
				(PKT_TYPE==IP_PKT_TYPE)?("IP"):("REPLY"),
				SRCIP
			);
	printf ("Sprintf valie: %s \n",MSGBUFF);
	addToMSGList(MSGBUFF,ERR,0);

	/* 
	 *	If the CoA isnt 0 then we will send it to the CoA address
	 *	because it means we are at the forreign host and it means
	 *	the CoA isnt at home
	 */	
	if (CoAMU)
	{
		/*
		 * If we want to send an IP PKT we must ensure that the 
		 * MN is registered or in a registered state before we can 
		 * complete the sending
		 */
		/*
		if (PKT_TYPE == IP_PKT_TYPE)
		{
			ForeignAgentsRegistered FAR = findFAHR(FAMobUnits,CoAMU);
			if (FAR.IsStillRegistered == TRUE)
			{
				MUsSendPort = (FRNNwk*1000)+CoAMU+PORT_CLEAR;
			}
			else
			{
				printf("MN IS NO LONGER REGISTERED CANT SEND IP PKT UNTIL REGISTRATION AGAIN\n");
				addToMSGList("MN IS NO LONGER REGISTERED CANT SEND IP PKT UNTIL REGISTRATION AGAIN",ERR,0);
				return ERR;
			}
		}
		else
		{
			MUsSendPort = (FRNNwk*1000)+CoAMU+PORT_CLEAR;
		}
		*/
		MUsSendPort = (FRNNwk*1000)+CoAMU+PORT_CLEAR;
	}
	else
	{
		/* 
		 *	if the CoA is zero it means that this packet is 
		 *	for the MU at the home agent so send it there !
		 *
		 *  Also before attempting to send the PKT ensure that
		 *  the MN has registered successfully 
		 */

		 int HMEintf = IP_checkDestinationInterface(PKT,IP_checkTunneledPkt(PKT));
		 /*
		 if (PKT_TYPE == IP_PKT_TYPE)
		 {
		 	HomeAgentsRegistered HR = findHAR(HAMobUnits,HMEintf);
			if (HR.IsFieldBeingUsed == TRUE)
			{
			  MUsSendPort = (FRNNwk*1000)+HMEintf+PORT_CLEAR;	
			}  
			else
			{
				printf("MN NO YET REGISTERED AT HA CANT SEND IP PKT\n");
				addToMSGList("MN NOT YET REGISTERED AT HA CANT SEND IP PKT",ERR,0);
				return ERR;
			}
		 }
		 else
		 {
		 	MUsSendPort = (FRNNwk*1000)+HMEintf+PORT_CLEAR;
		 }
		 */
		 MUsSendPort = (FRNNwk*1000)+HMEintf+PORT_CLEAR;
	}
#ifdef DEBUG_ME	
	printf("THIS IS THE PORT WE ARE SENDING IT TO %d\n",MUsSendPort);
#endif
	
	MIPPROTO_PackPacket(PKT,PKTbuff);
	sleep(WAIT_PERIOD_B4_SEND);
	
	if (DispatchPacket
			(
				DISPATCH_PACKET_WRITER,
				MUsSendPort,
				"127.0.0.1",
				PKTbuff
			) == ERR) 
	{
#ifdef DEBUG_ME
		printf("ERROR WHEN DISPATCHING TO Mobile Unit [ MU %d ] .....\n",
											MUsSendPort);
#endif
		strcpy(MSGBUFF,"MN NOT AT EXPECTED LOCATION, CANNOT SEND PKT");
		addToMSGList(MSGBUFF,ERR,0);
		return (ERR);
	}
	addToMSGList("SENDING OF DATA PKT TO MN SUCCESFUL",ERR,0);
	return 0;
}

/*
 * This function handles the re-routing of the packets according to the
 * routing table, what ever is in the routing table is followed by this 
 * function
 *
 * all this function does is look in the packet, look at the source address
 * dest address and then re-route the packet, it does this for all the diff
 * types of packets
 */
int ROUTER_DoRouting(PACKET_size_t PKT,int PKT_TYPE)
{
	int NET_ID = 0;
	int SRC_AD = 0;
	int DEST_ID= 0;

	char MSGBUFF[200];
	
	/* Handle request packet types */
	if (PKT_TYPE == REQT_PKT_TYPE)
	{
		SRC_AD = REQT_checkSourceAddressOfPacket(PKT);
		NET_ID = ROUTER_SWFC_rerouteRQSTPKT(ROUTERTable,PKT);
		DEST_ID= REQT_checkDestinationAddressOfPacket(PKT);

		sprintf(MSGBUFF,"ROUTING REQST PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d]",
				SRC_AD,NET_ID,DEST_ID);
		printf("ROUTING REQST PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d]",
				SRC_AD,NET_ID,DEST_ID);
	}
	/* Handle reply packet types */
	else if (PKT_TYPE == REPLY_PKT_TYPE)
	{
		SRC_AD = REPLY_checkSourceAddressOfPacket (PKT);
		DEST_ID= REPLY_checkDestinationAddressOfPacket(PKT);
		NET_ID = ROUTER_SWFC_rerouteREPLYPKT(ROUTERTable,PKT);

		sprintf(MSGBUFF,"ROUTING REPLY PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d]",
				SRC_AD,NET_ID,DEST_ID);
		printf("ROUTING REPLY PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d]",
				SRC_AD,NET_ID,DEST_ID);
	}
	/* handle ip pkt types */
	else if (PKT_TYPE == IP_PKT_TYPE)
	{
		SRC_AD = IP_checkSrcNwk(PKT,IP_checkTunneledPkt(PKT));
		DEST_ID= IP_checkDestNwk(PKT,IP_checkTunneledPkt(PKT));
		NET_ID = ROUTER_SWFC_rerouteIPPKT(ROUTERTable,PKT);

		sprintf
		(MSGBUFF,"ROUTING IP PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d] TUNNELED=[%s]",
			    SRC_AD,NET_ID,DEST_ID,(IP_checkTunneledPkt(PKT)?("TRUE"):("FALSE")));
		printf("ROUTING IP PKT FROM SRC NWK:[N%d] TO NHOP NWK:[N%d] FINAL DEST:[N%d]",
				SRC_AD,NET_ID,DEST_ID);
    }		
	addToMSGList (MSGBUFF,ERR,0);
	
	/*
	 * If the sending of the packet to the network failed it will be determined here 
	 */
	if(ROUTER_DispatchPacketWait (PKT,NET_ID) == ERR)
		printf ("Dispatching 0x%x To network N%d FAILED\n",PKT,NET_ID);
	else
		printf ("Dispatching 0x%x To network N%d SUCCESS\n",PKT,NET_ID);
	return 0;
}

/*
 *	Will simple de-register the mobile unit from the
 *	foreign agent here
 */
int ROUTER_DeRegisterMOBUNITAsFrgnHere(PACKET_size_t REQ_PKT)
{
		int CoA = REQT_checkCareOfAddress(REQ_PKT);

		printf("ROUTER_DeRegisterMOBUNITAsFrgnHere HAS BEEN CALLED \n");
		FAMobUnits->FAHArray[CoA].IsFieldBeingUsed  = FALSE;
		FAMobUnits->FAHArray[CoA].IsStillRegistered = FALSE;
		return 0;
}

/*
 * Will register the mobile unit in the HA table on this
 * current router
 */
int  ROUTER_RegisterMOBUNITAsHomeHere(PACKET_size_t PKT)
{
	setHAR(HAMobUnits,PKT);	
	return 0;
}

/*
 * Will register the mobile uni in the FA table on this
 * current router
 */
int  ROUTER_RegisterMOBUNITAsFrgnHere(PACKET_size_t PKT)
{
	int ret = setFAHR(FAMobUnits,PKT);
	return ret;
}
