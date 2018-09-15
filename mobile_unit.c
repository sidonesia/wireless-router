
/*------------------------------------------------------------------------
 *	This is the Code that represents the MN.
 *	The MN simply changes its addresses according to the different types
 *	of advertisments that it has.
 *
 *	The MN physically has 2 interfaces so in the code there are physcially
 *	2 ports to represent the 2 interfaces, one being the HmA and the other
 *	being the CoA.
 *-------------------------------------------------------------------------
 *	WRITTEN BY: SIDNEI BUDIMAN (c)
 *------------------------------------------------------------------------*/

#include "mobile_unit.h"
#include "mobile_unit_icmp.h"
#include "mobile_unit_request.h"

/*  REMINDER
	we might need to multiply the mobile units
	lifetime by 10 so that it longer and gives the
	mu to receieve pakts before its no longer registered 
 */



/*
 * This function will init all of the mobile units global variables
 * to some default value
 * It will also construct the objects and structures required to
 * store running data
 */

int MOBUNT_Init(char* configFile)
{
		MUnit = (MobileUnit_t*)malloc(sizeof(MobileUnit_t));
		
		initMobUnitPath();   /* set the path of the MU to no path yet */
		/* This will read the MN's config file */
		ScanMUConfigFileFirst(configFile);

		MUnit->HaveIRegistered   					= FALSE;
		MUnit->AmIRegistered     					= FALSE;
		MUnit->WasPreviouslyRegs 					= FALSE;
		MUnit->AmIMoving                            = FALSE;
		execute                  					= FALSE;

		MUnit->AmIAtHomeNow      					= TRUE;
		MUnit->HomeInterfaceUp   					= TRUE;
		MUnit->LifeTimeOfReg                        = 0;
		MUnit->waitFailedCount                      = 0;
		/* INITIALLY WE DONT START THE CoA INTERFACE because it intially starts at home */
		MUnit->COAInterfaceUp    					= FALSE; 

		MUnit->FrgnNetwork            				= COA_DISABLED; 
		MUnit->FrgnNetworkRange       				= ThisMUConfig.NwkMultFrgn; 
		MUnit->CurrentNetworkPort 					= MOBUNT_GetInterfaceAddressFromDHCP("CoA");
		strcpy(MUnit->MobUnitHost,ThisMUConfig.IPAddrHost);
		
		MUnit->previousCareOfAddress 				= 0;
		MUnit->HAgentDetails.MobUnitHomeAgentNwk    = ThisMUConfig.MUHmeNwk;
		MUnit->HAgentDetails.MobUnitRange           = ThisMUConfig.NwkMultHme;

		/* We will get a HomeAddress initially from a DHCP Server */
		MUnit->HAgentDetails.MobUnitHomeAgentPort   = MOBUNT_GetInterfaceAddressFromDHCP("HmA");
		strcpy(MUnit->HAgentDetails.MobUnitHomeAgentHost,ThisMUConfig.IPAddrHost);
		MUnit->RouterRanges 						= ThisMUConfig.RtrRange;
								  
		/* lets create the connection channel for both the interfaces */
		CONNECTChanHomeAddress  					= new_RCChan();
		CONNECTChanCareOfAddress 					= new_RCChan();
		
		/*INIT THE RANDOM SEED  This is used to asign a port number randomly for the CoA or HA */
		srand((unsigned int)time(NULL));
		
		/*
		 * This function will bring up the interfaces for the Mobile UNIT
		 * then once it has done that it will proceed to move
		 */
		if(MOBUNT_BringUpAllInterfaces() != ERR)
		{
			while(TRUE)
				if (MOBUNT_UnitMove() == FALSE)
					break;
		}
		return 0;
}

/*--------------------------------------------------------------------
 * This function will bring up both the threaded interfaces 
 * It will bring them up then sleep the main thread. all functions
 * triggered will be from the seperate threads
 --------------------------------------------------------------------*/
int MOBUNT_BringUpAllInterfaces()
{
	pthread_t CoADispatchThread;
	pthread_t HMADispatchThread;
	char      timeBUFF[40];
	
	/*
	 * Only bring up the home interface if
	 * Up is == to true For the Home interface, we dont want to bring
	 * it up if we are at a foreign network
	 */
	if(MUnit->HomeInterfaceUp)
	{
			/* bring up HMA interface */
			if (
				pthread_create
				(
				    &HMADispatchThread,NULL,
					MOBUNT_BringUpHomeAddress,NULL
				 ) == ERR
			   )
			{
				getTimeNow(timeBUFF);
				printf("[MOBILE UNIT]@[%s]: MOBUNT_BringUpHomeAddress Failed ... \n",timeBUFF);
				return ERR;
			}
	}

	/*-----------------------------------------------------
	 * Will bring up the home interface if and only if 
	 * we allow it to IE we are at a foreign network
	 *----------------------------------------------------*/
	if (MUnit->COAInterfaceUp)
	{
			if (
				pthread_create
				  (
				    &CoADispatchThread,NULL,
					MOBUNT_BringUpCareOfAddress,NULL
				  ) == ERR
			   )
			{
				printf("[MOBILE UNIT]@[%s]: MOBUNT_BringUpCareOfAddress Failed ... \n",timeBUFF);
				return ERR;
			}
	}
	return 0;
}

/*----------------------------------------------------------------
	This function will set up the correct procedures for setting
	up the interface which will act as the home address of this
	mobile unit
 -----------------------------------------------------------------*/
void* MOBUNT_BringUpHomeAddress(void* param)
{
	char      IPBUFF[40];
	char      timeBUFF[40];

	/* Its the Network Its on, Multiplyed By 1000 + PORT_CLEAR
	 * Plus the interface that the mobile Unit is currently listening
	 * on
	 */
	MUnit->DualIf.HMA_IFace = MUnit->HAgentDetails.MobUnitHomeAgentNwk * /* 1 - 5 */
				  MUnit->HAgentDetails.MobUnitRange +          		     /* 1000  */
				  PORT_CLEAR +  /* some number i define so that it isnt to low a port number */
				  MUnit->HAgentDetails.MobUnitHomeAgentPort; 		     /* 1- 15 */
	
	/*
	 * Will setup the connection channel.
	 * For the HomeInterface
	 */
	InitRouterChannel(CONNECTChanHomeAddress, MOBUNT_CallbackHA);	

	/*
	 * This while loop is very important. It will attempt to find a free
	 * HA address, if it is assigned an address which is taken then it will
	 * say NO ! and loop around again to get another one until it gets one
	 * that it can bind to, IE not used
	 */
	while (TRUE)
	{
		/*
		 * If this evaluates to true it means the address has been taken
		 * BY another mobile unit
		 */
	    if(InitServerSocket 
				(
					 CONNECTChanHomeAddress,
					 MUnit->MobUnitHost,
					 MUnit->DualIf.HMA_IFace
				) == BIND_ERR)
		{
				/* try a different address !! means been taken */
				convertPortToIpFormat
				(
					MUnit->HAgentDetails.MobUnitHomeAgentNwk,
					MUnit->HAgentDetails.MobUnitHomeAgentPort,
					IPBUFF
				);
				getTimeNow(timeBUFF);
				printf("[MOBILE UNIT]@[%s]: HmA %s IS TAKEN CHOOSING ANOTHER\n",timeBUFF,IPBUFF);
				MUnit->HAgentDetails.MobUnitHomeAgentPort 
						= MOBUNT_GetInterfaceAddressFromDHCP("HmA");

				/* Re-create the home interface */
				MUnit->DualIf.HMA_IFace = MUnit->HAgentDetails.MobUnitHomeAgentNwk * /* 1 - 5 */
			  				MUnit->HAgentDetails.MobUnitRange +          		     /* 1000  */
			  				PORT_CLEAR +
			  				MUnit->HAgentDetails.MobUnitHomeAgentPort; 		         /* 1- 15 */
		}
		else
		{
				break;
		}
	}
#ifdef DEBUG_ME
	printf
		(
			"[MOBILE UNIT]: HMA NOW LISTENINING ON PORT %d ON HOST %s\n",
			MUnit->DualIf.HMA_IFace - PORT_CLEAR,
			MUnit->MobUnitHost
		);	
#else
	getTimeNow(timeBUFF);
	convertPortToIpFormat(
				MUnit->HAgentDetails.MobUnitHomeAgentNwk,
				MUnit->HAgentDetails.MobUnitHomeAgentPort,
				IPBUFF
				);
	printf("[MOBILE UNIT]@[%s]: HMA INTERFACE STARTED ON ADDRESS %s\n",timeBUFF,IPBUFF);
#endif
	/*
	 * Function will continue to run until the Home interface is told
	 * To shutdown by the HmA shutdown function
	 */
	while(MUnit->HomeInterfaceUp)
	{		
		WaitForEvents
		(
			CONNECTChanHomeAddress,
			/* 
			 * obsolete parameter, i was going to use this for somthing but
			 * didnt have time to, just leave it here
			 */
			WRITE_BACK_TO_SOCK_SERVER
		);
	}

	/*
	 * If we exit this loop we will kill the socket so that
	 * next time the MN goes back home the socket will be 
	 * OK to bind to
	 */
	close(CONNECTChanHomeAddress->ServerSocket);
	return 0;
}

/*--------------------------------------------------------*
 * Trigger function to send a dummy packet to the CoA     *
 * What this alows to happen is it will allow the loop to * 
 * fall through and re-bind the CoA on another socket     *
 *--------------------------------------------------------*/
int MOBUNT_TriggerChangeInCoA()
{
	MOBUNT_DispatchPacketWait
				(
					MUnit->DualIf.CoA_IFace,
					0
				);
	return 0;
}

/*----------------------------------------------------------*
 * Function to shutdown the CoA                             *
 * Sets the CoAIntefaceFlag to false and then triggers a    *
 * change in the CoA so when the loop falls through it does *
 * not rebind and hence the CoA is shutdown                 *
 *----------------------------------------------------------*/ 
int MOBUNT_ShutdownCoA()
{
	char timeBUFF[40];

	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: SHUTTING DOWN THE CARE(CoA) INTERFACE: Reason ? Position = Home Network \n",timeBUFF);
	printf("[MOBILE UNIT]@[%s]: CHANGING CoA TO ADDRESS: 0.0.0.0\n",timeBUFF);
	MUnit->COAInterfaceUp = FALSE;
	MOBUNT_TriggerChangeInCoA();
	return 0;
}

/*------------------------------------------------------------------*
 * Trigger function to send dummy packet back to the Home Address   *
 * EXACTLY THE SAME AS WHAT HAPPENS ON CoA EXCEPT THIS IS FOR HMA   *
 *------------------------------------------------------------------*/
int MOBUNT_TriggerChangeInHMA()
{
	MOBUNT_DispatchPacketWait
				(
					MUnit->DualIf.HMA_IFace,
					0
				);
	return 0;
}

/*------------------------------------------------------------*
 * This function will end the thread that allows the 
 * Home Address to recieve packets. This shall be envoked
 * When we move to a foreign network, when we move to a 
 * foreign network we no longer need this address active
 *------------------------------------------------------------*/
int MOBUNT_ShutdownHMA()
{
	char timeBUFF[40];

	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: SHUTTING DOWN THE HOME(HmA) INTERFACE:Reason ? Position=Foreign Network \n",timeBUFF);
	printf("[MOBILE UNIT]@[%s]: CHANGING HmA TO ADDRESS 0.0.0.0 \n",timeBUFF);
	MUnit->HomeInterfaceUp = FALSE;
	MOBUNT_TriggerChangeInHMA();
	return 0;
}

/*----------------------------------------------------------
 * Get an address somewhere between 1 & 14 from the 
 * DHCP Server. This is the specific host address 
 *
 * Simple random number generator between 1 and 15 ,not to much effort not
 * part of marking scheme
 -----------------------------------------------------------*/


 /* check if 15 is dispachted to !!! important */
int MOBUNT_GetInterfaceAddressFromDHCP(char* inf)
{
	char timeBUFF[40];

	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: GETTING A TMP ADDRESS FROM THE DHCP SERVER: FOR THE %s INTERFACE\n",timeBUFF,inf);
	return ((rand() % 14)+1);
}

/*-------------------------------------------------------------------------
 * This function will bring up the care of address interface, it simply
 * just sets up the correct values and ports so it knows how to listen
 * for packets
 *
 *
 * Everything that happens here is basically the same as what happens in the
 * HmA bringup Interface
 --------------------------------------------------------------------------*/
void* MOBUNT_BringUpCareOfAddress(void* param)
{
	char IPBUFF[30];
	char timeBUFF[40];
	/*
	 * Put the whole thing in a while loop !! why ? because if the CoA
	 * changes we can re-bind the socket again to listen to another port
	 * This simulates a changing of care of address
	 *
	 * But why all in a while loop ? so that we can force a change of 
	 * Address by sending a dummy packet back to itself
	 */
	while (MUnit->COAInterfaceUp)
	{
	  MUnit->DualIf.CoA_IFace = MUnit->FrgnNetwork *         /* 1 - 5  */
	  							MUnit->FrgnNetworkRange +    /* 1000   */
								PORT_CLEAR +
	  							MUnit->CurrentNetworkPort;   /* 1 - 15 */
#ifdef DEBUG_ME	
		printf ("AFTER THE COA IS TRIGGERED ITS %d \n",MUnit->DualIf.CoA_IFace);
#endif

	  /*
	   * If we recieve a packet and the COA is the same as before dont worry 
	   * about doing any rebindings, however if they are different then we need
	   * to do it again, because what this means is that the mobile unit
	   * has moved to a new network
	   */
	  
	  if (MUnit->DualIf.CoA_IFace != MUnit->previousCareOfAddress)
	  {
			/*
			 * Dont forget to close the socket before re-binding, if we 
			 * Need to go to this network again and the socket isnt closed
			 * properly we will have issues with rebinding :-(
			 */
			close(CONNECTChanCareOfAddress->ServerSocket);
  	  		InitRouterChannel(CONNECTChanCareOfAddress, MOBUNT_CallbackFA);

			while(TRUE)
			{
				if(InitServerSocket
					(
						 CONNECTChanCareOfAddress,
						 MUnit->MobUnitHost,
						 /* 7000 + 5 or 7000 = 4 etc, router ports */
						 MUnit->DualIf.CoA_IFace
					) == BIND_ERR 
				  )
				{
					/* try a different address !! means been taken */
					MUnit->tmpAddrTries++;

					convertPortToIpFormat
					(
							MUnit->FrgnNetwork,
							MUnit->CurrentNetworkPort,
							IPBUFF
					);
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: CoA %s IS TAKEN CHOOSING ANOTHER\n",timeBUFF,IPBUFF);
					MUnit->CurrentNetworkPort = 
							MOBUNT_GetInterfaceAddressFromDHCP("CoA");
					/* Now re-create the care of address */
					MUnit->DualIf.CoA_IFace = MUnit->FrgnNetwork *         /* 1 - 5  */
							MUnit->FrgnNetworkRange +    				   /* 1000   */
							PORT_CLEAR +
							MUnit->CurrentNetworkPort;   				   /* 1 - 15 */
				}
				else
				{
					break;
				}
			}
			/* Assign the previous care of address so we can use it to recheck
			 * to see if the care of address has changed 
			 * */
			MUnit->previousCareOfAddress = MUnit->DualIf.CoA_IFace;
		}
#ifdef DEBUG_ME
	  	printf
			(
				"[MOBILE UNIT]@[%s]: COA NOW LISTENING ON PORT %d ON HOST %s\n",
				timeBUFF,
				MUnit->DualIf.CoA_IFace - PORT_CLEAR,
				MUnit->MobUnitHost
			);	
#else
		convertPortToIpFormat(
					MUnit->FrgnNetwork,
					MUnit->CurrentNetworkPort,
					IPBUFF
				);
#endif
		/*
		 * The event watcher, will be triggered everytime we get 
		 * A packet , ie and event
		 */
		WaitForEvents
		(
			CONNECTChanCareOfAddress,
			WRITE_BACK_TO_SOCK_SERVER
		);
	}
	return 0;
}


/*----------------------------------------------------------*
 * As usual this is just a general dispatch function.       *
 * First parameter is the port we want to dispatch it to    *
 * Second parameter is the packet.                          *
 *----------------------------------------------------------*/
int MOBUNT_DispatchPacketWait
		(
			/* 
			 * 	This port number will be the port number of the router that 
			 *	acts as a gateway for this network
			 */
			int RoutersPortNumber,
			PACKET_size_t PKT
		)
{
	char  PKTbuff[20];	

	MIPPROTO_PackPacket(PKT,PKTbuff);
	sleep(WAIT_PERIOD_B4_SEND);

	if (DispatchPacket
			(
				DISPATCH_PACKET_WRITER,
				RoutersPortNumber,
				"127.0.0.1", /* for all intensive purposes its cool */
				PKTbuff
			) == ERR
		)
	{
		printf ("Router with port RoutersPortNumber %d not found \n",
				RoutersPortNumber);	
	}
	return 0;
}

/*-------------------------------------------------------------------
 *	- Will get the the RQST packet format when sending to the HA    *
 *	- Will then find the port number for where the HA listens       *
 *	- Will dispatch this RQST packet to the correct port to the HA  *
 -------------------------------------------------------------------*/
int MOBUNT_RegisterWithHomeAgent()
{
	/*
	 * Here we make the request packet for the routers
	 *
	 *
	 */
	PACKET_size_t REQST_PKT = MUNITREQ_makeREQPKTSForHomeAgent
								(
									MUnit
								);
	MOBUNT_DispatchPacketWait
				(
					MUnit->HomeAddressRouterPort,
					REQST_PKT
				);
	return 0;
}

/*-------------------------------------------------------------------
 *	- Will get the the RQST packet format when sending to the FA    *
 *	- Will then find the port number for where the FA listens       *
 *	- Will dispatch this RQST packet to the correct port to the FA  *
 -------------------------------------------------------------------*/
int MOBUNT_RegisterWithFrgnAgent()
{
	PACKET_size_t REQST_PKT = MUNITREQ_makeREQPKTSForForeignAgentReg
								(
									MUnit
								);
	MOBUNT_DispatchPacketWait
				(
					MUnit->FrgnAddressRouterPort,
					REQST_PKT
				);
	return 0;
}

/*-------------------------------------------------------------------*
 * Will de register this host from the FA and tell the global        *
 * amIRegistered to false                                            *
 *                                                                   *
 * This function is obsolete, we no longer need to say de-register   *
 * because the routers do that automatically                         *
 *-------------------------------------------------------------------*/
int MOBUNT_DeRegisterWithFrgnAgent()
{
	/*
	 * We shall formulate the REQst packet used when we
	 * want to de-register from the Foreign agent
	 */
	char          timeBUFF[40];
	PACKET_size_t REQST_PKT = MUNITREQ_makeREQPKTSForForeignAgentDeReg
								(
									MUnit
								);
	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: CALL FOR DE-REGISTERING FROM N%d\n",timeBUFF,MUnit->FrgnNetwork);

	/* Since we are de-registered we are no longer registered so we tell ourselfves that */
	MUnit->AmIRegistered = FALSE;

	/* Now send this de-registration packet to the router */
	MOBUNT_DispatchPacketWait
				(
					MUnit->FrgnAddressRouterPort,
					REQST_PKT
				);
	return 0;
}

/*----------------------------------------------------*
 * Does the sending of the IP pkts                    *
 * first it checks if the current mobile unit         *
 * is registered or not though, cuz if its not        *
 * it does not allow the IP pkt to be sent            *
 -----------------------------------------------------*/
int MOBUNT_sendIPPacket
            (
			   int Network,
			   int Interface
			)
{
		// dont hardcode the lifetime
		time_t        time_now;
		PACKET_size_t IP_PKT ;
		int           DispatchPort  = 0;
		char          timeBUFF[40];

		time(&time_now);
		/* if it uses this first then it will keep the same CoA */
		if((time_now - MUnit->RegisterTime) >= LIFETIME)
		{
			if(MUnit->AmIAtHomeNow == FALSE)
			{
					MUnit->AmIRegistered = FALSE;
					MOBUNT_RegisterWithFrgnAgent();
			}
		}
		/* we just do a busy wait until registration is approved AmIRegistered is set to TRUE */
		if (MUnit->AmIAtHomeNow == FALSE)
		{
			if (MUnit->AmIRegistered == FALSE)
			{
				getTimeNow(timeBUFF);
				printf("[MOBILE UNIT]@[%s]: IP PKT CANT BE SENT BECAUSE MN IS NO LONGER REGISTERED\n",timeBUFF);
				return ERR;
			}
		}
		// sendTheIPpacketNow();

		IP_PKT       = MOBUNT_makeIPPacket(Network,Interface);

		/*
		 * Forumlate the port we need to send the pkt to. This will be the gateway
		 * router port for this current network. So for N5 this port will be the port
		 * the router for N5 will be listening on
		 */
		 if (MUnit->AmIAtHomeNow == FALSE)
		 {
		 	getTimeNow(timeBUFF);	
			DispatchPort = MUnit->RouterRanges + MUnit->FrgnNetwork;
			printf ("[MOBILE UNIT]@[%s]: SENDING A PACKET TO GATEWAY ROUTER ON NEWTORK %d\n",
					timeBUFF,
					MUnit->FrgnNetwork);
		 }
		 else
		 {
		 	getTimeNow(timeBUFF);	
		 	DispatchPort = MUnit->RouterRanges + MUnit->HAgentDetails.MobUnitHomeAgentNwk;
			printf ("[MOBILE UNIT]@[%s]: SENDING A PACKET TO GATEWAY ROUTER ON NEWTORK %d\n",
					timeBUFF,
					MUnit->HAgentDetails.MobUnitHomeAgentNwk);
		 }

		
		/*
		 * Now we simply send this pkt
		 */
		MOBUNT_DispatchPacketWait
				(
				 DispatchPort,
				 IP_PKT
				);
		return 0;
}


/*----------------------------------------------------------*
 * This fuction creates the packets ip packets              *
 * The destiniation address is specified by the parameters  *
 * whilst the source address is obviously always the home   *
 * address of this mobile unit                              *
 *----------------------------------------------------------*/
int MOBUNT_makeIPPacket(int destNwk,int destIf)
{
	PACKET_size_t IP_PKT = 0;	
	
	/* define what type of packet this is, this being an IP packet */
	IP_PKT = MIPPROTO_putPacketType
			 (
			   IP_PKT,
			   IP_PKT_TYPE
			 );
	
	/* put the source network, will be the network where by the home agent is situatied*/
	IP_PKT = IP_putSrcNwk
			 (
			   IP_PKT,
			   MUnit->HAgentDetails.MobUnitHomeAgentNwk,
			   FALSE
			 );
			 
		/* put the CoA address */	
	if(MUnit->AmIAtHomeNow == FALSE)
		IP_PKT = IP_putSrcNwk
				 (
				   IP_PKT,
			   	   MUnit->FrgnNetwork,
			   	   TRUE
			 	 );
	else
		IP_PKT = IP_putSrcNwk
				 (
				   IP_PKT,
			   	   0,
			   	   TRUE
			 	 );
		
	/* put the source address of this packet will be home address*/
	IP_PKT = IP_putSourceInterface
			 (
			   IP_PKT,
			   MUnit->HAgentDetails.MobUnitHomeAgentPort,
			   FALSE
			 );
	/* put the CoA interface*/	
	IP_PKT = IP_putSourceInterface
			 (
			   IP_PKT,
			   MUnit->CurrentNetworkPort,
			   TRUE
			 );
	/* put the specific hosts address */
	IP_PKT = IP_putDestinationInterface
			 (
			   IP_PKT,
			   destIf,
			   FALSE
			 );
	/* put the address desination network */
	IP_PKT = IP_putDestNwk
			 (
			   IP_PKT,
			   destNwk,
			   FALSE
			 );
	return (IP_PKT);
}


/*------------------------------------------------------------
 * This is the callback function for anything that
 * happens on the HomeAddress Interface
 *
 * The HA only listens to ICMP's and IP packets
 * All other types of pkts are not handled
 *------------------------------------------------------------*/
int MOBUNT_CallbackHA (PACKET_send_t RawPacket)
{
		PACKET_size_t PKT = MIPPROTO_unPackPacket(RawPacket);	
		char          timeBUFF[40];
		
		switch (MIPPROTO_getPacketType(PKT))
		{
			case ICMP_PKT_TYPE  :
			{
				PACKET_size_t FLAG_SEG = ICMP_checkICMPFlags(PKT);
				PACKET_size_t AGNTType = ICMP_checkAgentType(FLAG_SEG);
				
				/*
				 * If the MN recives a HA advertisment and at the 
				 * current moment it isnt registered then we shall inform
				 * this MN to register with the HA, we also tell the MN
				 * to take note of who its HOME AGENT IS and to set itself
				 * to being registered
				 */
				if(AGNTType == ICMP_AGT_HME && MUnit->HaveIRegistered == FALSE)
				{
					/* will tell this mobile unit who the home agent is */
					/* and set it in the MU's database */
					MUNIT_ICMP_setHomeAgent(MUnit,PKT);
					MOBUNT_RegisterWithHomeAgent();
					MUnit->HaveIRegistered = TRUE;
				}
			}
			break;

			case IP_PKT_TYPE:
				{
					/*
					 * If we get an IP pkt simply just print it out saying we
					 * Got one, there is no need to do anything else with it
					 */
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: AN IP PKT 0x%x WAS JUST RECIVED ON THE HA \n",timeBUFF,PKT);
					printf("[MOBILE UNIT]@[%s]: NOW MN REPLY BACK FROM HA INTERFACE TO SOURCE\n",timeBUFF);
					MOBUNT_ReplyIPPkt(PKT);
				}
				break;
			default :
				/*
				 * If its a non standard pkt BUT it has a value of 0x0 then we know
				 * it was used to trigger the interface so simply print out
				 */
				if (PKT == 0x0)
				{
#ifdef DEBUG_ME
					printf("[MOBILE UNIT]: TRIGGERING HOME ADDRESS INTERFACE TO CHANGE STATE\n");
#endif
				}
				else
					printf("PKT WAS NOT RECOGNIZED \n");
				break;
		}
		return 0;
}

/*---------------------------------------------------------
	This is the callback function for anything that
	happens on the Care Of Address interface

	Will mainly be listening to icmp packets to register
	its self and also listen on reply packets at registration
	time
 ----------------------------------------------------------*/
int MOBUNT_CallbackFA (PACKET_send_t RawPacket)
{
	PACKET_size_t PKT = MIPPROTO_unPackPacket(RawPacket);	
	char          timeBUFF[40];
#ifdef DEBUG_ME		
		printf("RECV PKT ON FA interface is : ");
		MIPPROTO_debugPacket(PKT);
#endif
	switch (MIPPROTO_getPacketType(PKT))
	{
		case ICMP_PKT_TYPE  :
			{
				/*
				 * What we do here is we try to get the foreign agents 
				 * ad packets, we check the flags in the icmp packet. if
				 * the ads are for HA's we ignore it but if its a FA and
				 * we are at a foreign network we will take it and process it
				 */
				PACKET_size_t FLAG_SEG = ICMP_checkICMPFlags(PKT);
				PACKET_size_t AGNTType = ICMP_checkAgentType(FLAG_SEG);
				if(AGNTType == ICMP_AGT_FGN)
				{
					
					if (MUnit->AmIMoving == TRUE)
					{
						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: IS CURRENTLY MOVING BETWEEN RTRS CANT ACCEPT ICMP PKTS\n",timeBUFF);
						return ERR;
					}
					
					/*
					 * First check to see if the MN is still registered if
					 * it is we can ignore the advertisment, if it isnt then
					 * we need to process it
					 */
					if (MOBUNT_StillRegistered() == FALSE)
					{
						/*
						 * Will simply check to see if it was previously registerd
						 * if it wasnt it will enter here, this is so we know how
						 * many times we MN has tried to register.
						 * An MN will not always register successfull the first time
						 * so its good to know how many times i needed to register.
						 * this issue arrises if there is more then 1 MN
						 */
						if(MUnit->WasPreviouslyRegs == FALSE && !MUnit->waitFailedCount)
						{
							getTimeNow(timeBUFF);
							printf("[MOBILE UNIT]@[%s]: FIRST TIME REGISTERING ON N%d\n",timeBUFF,MUnit->FrgnNetwork);
							MUNIT_ICMP_setFrgnAgent(MUnit,PKT);
							MUnit->CurrentNetworkPort = ICMP_checkAvailCareOfAddress(PKT,ICMP_COA_1);
						}
						/*
						 * This is triggered if and only if the MN was uncessful at registering on the
						 * first time before another icmp packet was recived
						 */
						else if (MUnit->waitFailedCount > 0 && MUnit->WasPreviouslyRegs == FALSE)
						{
							getTimeNow(timeBUFF);
							printf
							("[MOBILE UNIT]@[%s]: REPLY PACKET NOT RECIVED FROM FA, ROUTERS COULD BE BUSY\n",timeBUFF);

							getTimeNow(timeBUFF);
							printf(
									"[MOBILE UNIT]@[%s]: ATTEMPT %d  REGISTERING ON N%d\n",
									timeBUFF,
									MUnit->waitFailedCount,
									MUnit->FrgnNetwork
								  );
							MUNIT_ICMP_setFrgnAgent(MUnit,PKT);
							MUnit->CurrentNetworkPort = ICMP_checkAvailCareOfAddress(PKT,ICMP_COA_1);
						}
						else
						{
							getTimeNow(timeBUFF);
							printf("[MOBILE UNIT]@[%s]: REGISTRATION ABOUT TO EXPIRE, BUT STILL ON N%d, RE-REGISTERING NOW\n",
							timeBUFF,
						    MUnit->FrgnNetwork);
						}
						MUnit->waitFailedCount++;
						/*
						 * This is now the address the Mn will listen on for the reply packet
						 * before it is officially registered
						 */
						MOBUNT_TriggerChangeInCoA();

						/*
						 * Now it proceeds to register its self with 
						 * The foreign agent once it has a listen port
						 * this pkt will be sent all the way to the HA and
						 * back, once it comes back it means the MN is officially
						 * registered
						 */
						MOBUNT_RegisterWithFrgnAgent();

						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: HAS ASKED FOR REG-BUT HAS NOT YET BEEN GIVEN\n",timeBUFF);
						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: WAITING FOR REPLY PACKET GIVING OFFICIAL COA \n",timeBUFF);
				}
				else
				{
						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: STILL REGISTERED, IGNORING ICMP ADVERT PKT\n",timeBUFF);
				}
			}
		}
		break;
		case REPLY_PKT_TYPE :
			{
				char IPBUFF[30];
				/*
				 * When a reply packet is recived it means that the MN is officially registered
				 * It means that the MN's location is know known by the HA.
				 * All we do here is print out that the MN is now registered and we set the
				 * AmIRegistered flag to TRUE because it is now registered
				 * 
				 * We also need to set the time the Registeration occured
				 */
				
				time_t time_now;
				char   timeBUFF[40];
				
				getTimeNow(timeBUFF);
				printf("[MOBILE UNIT]@[%s]: HAS RECIVED THE REPLY DATA 0x%x AND IS NOW REGISTERED \n",timeBUFF,PKT);
				convertPortToIpFormat
				(
					MUnit->FrgnNetwork,
					MUnit->CurrentNetworkPort,
					IPBUFF
				);
				getTimeNow(timeBUFF);
				printf ("[MOBILE UNIT]@[%s]: WE ARE NOW ON NETWORK %d \n",timeBUFF,MUnit->FrgnNetwork);
				getTimeNow(timeBUFF);
				printf("[MOBILE UNIT]@[%s]: THE OFFICIAL CoA IS: %s \n",timeBUFF,IPBUFF);
				
				MUnit->AmIRegistered     = TRUE;
				MUnit->WasPreviouslyRegs = TRUE;
				
				/* lets set the actual time that registeration was accepted by the network */
				time(&time_now);
				MUnit->RegisterTime = time_now;
			}
			break;
			
			/*
			 * This is the IP pkt that is sent by the CN and MN if we get an IP pkt we
			 * simply just print it out. That we got an IP pkt
			 */
		case IP_PKT_TYPE :
			{
				char timeBUFF[40];
				if (MUnit->AmIRegistered == TRUE)
				{
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: AN IP PKT WAS JUST RECIVED ON THR CoA: 0x%x \n",timeBUFF,PKT);	
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: NOW MN REPLY BACK FROM FA INTERFACE TO SOURCE\n",timeBUFF);
					MOBUNT_ReplyIPPkt(PKT);
				}
				else
				{
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: AN IP PKT WAS RECVD'd BUT CANT PROCESS NO-LONGER REGD'd\n",timeBUFF);	
				}
			}
			break;
		
		default :
			/* its a trigger packet */
			if (PKT == TRIGGER_PKT)
			{
#ifdef DEBUG_ME
				printf("[MOBILE UNIT]: TRIGGERING CARE OF ADDRESS INTERFACE TO CHANGE STATE\n");
#endif
			}
			else
				printf("PKT WAS NOT RECOGNIZED \n");
			break;
	}
	return 0;
}

/*
 * This function will determine if the mobile unit is still
 * Registered or not, it will check the time between the 
 * reception of the reply packet and the time now to see if
 * the lifetime has passed
 *
 * If time has passed then false is returned because StillRegistered
 * is not TRUE :-)
 */
int   MOBUNT_StillRegistered()
{
		time_t time_now;
		time(&time_now);

		if (MUnit->LifeTimeOfReg == 0)
				return (FALSE);
		if ((time_now - MUnit->RegisterTime) >= MUnit->LifeTimeOfReg)
		//if ((MUnit->LifeTimeOfReg - (time_now - MUnit->RegisterTime)) < 4)
		{
				return (FALSE);
		}
		return (TRUE);
}

/* This function will reply back to the sender */
int   MOBUNT_ReplyIPPkt(PACKET_size_t PKT)
{
	/*
	 * we always want the original address not the HA address so
	 * we know where to reply to 
	 */
	int  SRCNwk  = IP_checkSrcNwk(PKT,FALSE);
	int  SRCIntf = IP_checkSourceInterface(PKT,FALSE);
	char timeBUFF[40];
	
	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: REPLY BACK TO NODE ON N%d ON INTERFACE:%d \n",timeBUFF,SRCNwk,SRCIntf);
	MOBUNT_sendIPPacket(SRCNwk,SRCIntf);

	return 0;
}

/*
 * This will determine how the Mobile Unit will move
 * It will move according to the configuration file and
 * Its residance time will also be according to the config
 * file
 */ 
int MOBUNT_UnitMove()
{
		int  ix = 0;
		char timeBUFF[40];
		
		for (ix =0;ix < MU_MAX_TRAVEL;ix++)
		{
			if (PATH_OF_MU[ix].validLocation == TRUE)
			{
			    /* if we are not at home then set the forign network field */
				if(
					PATH_OF_MU[ix].CurrentNetwork != 
					MUnit->HAgentDetails.MobUnitHomeAgentNwk
				  )
				{
					MUnit->AmIAtHomeNow = FALSE;

					getTimeNow(timeBUFF);
					printf("-------------------------------------\n");
					printf("*[MOBILE UNIT]@[%s]: MOVING TO [N%d]*\n",timeBUFF,PATH_OF_MU[ix].CurrentNetwork);
					printf("-------------------------------------\n");
					MUnit->FrgnNetwork = PATH_OF_MU[ix].CurrentNetwork;	
					/* Will inform the CoA to get a Temporary Address from DhCP */
					MUnit->waitFailedCount = 0;

					/* This trigger is to tell the MU that its no a different network !!*/
					MOBUNT_TriggerChangeInCoA();
					MUnit->AmIMoving = FALSE;
					
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: WAITING FOR REGISTRATION\n",timeBUFF);
					/* busy wait whilst we wait for registraion to be done */
					while(MUnit->AmIRegistered == FALSE);
					
					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: REG COMPLETE NOW RESIDING \n",timeBUFF);
					/* we stay on this network for this amount of time */
					
					/* WE DONT NEED THIS PART BUT JUST KEEP IT HERE
					if (PATH_OF_MU[ix].CurrentNetwork == 5)
					{
							//  This part needs to be configurable
							printf("[MOBILE UNIT]: MOBILE UNIT ABOUT TO SEND TO 2,7 \n");
							MOBUNT_sendIPPacket(2,7);
					}
					*/

					sleep (PATH_OF_MU[ix].ResidenceTime);
					MUnit->AmIMoving = TRUE;
					/*
					 * If we wanted to de-register we would do it here but since
					 * the agents handle it automatically now we dont need to do
					 * it explicitly
					 */
				     // ACTUALLY !!!! ..... Lets De-register	
					/*
					 * If we move before the lifetime is up we will explicity call de-reg
					 * if we move after lifetime is up no need to call because router would
					 * have already set the MN to not be registered
					 */
					 if (MUnit->AmIRegistered == TRUE)	 
					 {
					 	getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: EXPLICIT CALL TO DE-REG FROM FA %d DONE \n"
								,timeBUFF,MUnit->FrgnNetwork);
						MOBUNT_DeRegisterWithFrgnAgent();
					 }
					MUnit->LifeTimeOfReg     = 0;
					MUnit->WasPreviouslyRegs = FALSE;
				}
				else
				{
					/*
					  if im back at the HA again set the frgn nwk and the coa to 0 !!
					 */


				   /*	
					* This part of the code will tell the MU that the
					* Mobile unit is at home so there is no need for a
					* Care of address, we shut this down
					*/

					MUnit->AmIAtHomeNow = TRUE;
					/*----------------------------------------------*/
					MUnit->HomeInterfaceUp = TRUE;
					if (MUnit->COAInterfaceUp)
						MOBUNT_ShutdownCoA();
					if(execute)
							MOBUNT_BringUpAllInterfaces();
					/*----------------------------------------------*/

					if (MUnit->HaveIRegistered == TRUE)
					{
						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: BACK TO HOME AGENT AGAIN\n",timeBUFF);
					}
					else
					{
						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: WAITING TO REG WITH HA\n",timeBUFF);
						while (MUnit->HaveIRegistered == FALSE);

						getTimeNow(timeBUFF);
						printf("[MOBILE UNIT]@[%s]: REGISTERED SUCCESSFULLY WITH HA\n",timeBUFF);
					}

					getTimeNow(timeBUFF);
					printf("[MOBILE UNIT]@[%s]: HOW RESIDING AT HA\n",timeBUFF);
					sleep (PATH_OF_MU[ix].ResidenceTime);
					
					/*
					 * This part of the code says we are now moving from 
					 * the Home Agent so we disable the HMA and bring up
					 * The care of address interface
					 */

					/*----------------------------------------------*/
					MUnit->COAInterfaceUp = TRUE;
					if (MUnit->HomeInterfaceUp)
						MOBUNT_ShutdownHMA();
					MOBUNT_BringUpAllInterfaces();
					/*----------------------------------------------*/
					
					execute = TRUE;
				}
				/* after moving to another network the MU is no longer registered */
				MUnit->AmIRegistered = FALSE;
			}
		}
		getTimeNow(timeBUFF);
		printf("[MOBILE UNIT]@[%s]: FINISHED MOVING \n",timeBUFF);
		return 0;
}
