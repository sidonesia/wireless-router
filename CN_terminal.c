#include "CN_terminal.h"
#include "mobile_unit.h"


int CNUnit_Init()
{
	CNunit = (CN_Unit*)malloc(sizeof(CN_Unit));
	CNunit->CNsListenPort = 0;

	CNunit->NetworkRouterPort = CNunit->RouterRanges + CNunit->Network;

	CONNECT_CNUnit = new_RCChan();
	return 0;
}

/*------------------------------------------------*
 * This is the callback function for the CN_Unit  *
 *------------------------------------------------*/
int CNUnit_Callback
	(
		PACKET_send_t RawPacket
	)
{
	PACKET_size_t PKT = MIPPROTO_unPackPacket(RawPacket);
	char IPBUFFMSG[50];

	switch (MIPPROTO_getPacketType(PKT))
	{
		case IP_PKT_TYPE :
				
				printf("IP PKT 0x%x RECIEVED \n",PKT);
				sprintf(IPBUFFMSG,"IP PKT 0x%x RECIEVED \n",PKT);
				addToMsgList(IPBUFFMSG,PKT);
				break;
		
		case  ICMP_PKT_TYPE :
				printf("ICMP PKT RECIEVED ON MY INTERFACE BUT IM NOT A MOBILE UNIT SO IGNORING\n ");
				break;
		default:
				printf("Unrecognised PKT Type\n");
				break;
	}
	return 0;
}

/*---------------------------------------------*
	We do not tunnel any of the packets here
	thats why all the parameters are false,
	We only tunnel them when they get to the 
	HA router 
 *---------------------------------------------*/
PACKET_size_t CNUnit_makeIPPKTs()
{
	PACKET_size_t IP_PKT = 0;

	IP_PKT = MIPPROTO_putPacketType
				(
					IP_PKT,
					IP_PKT_TYPE
				);
	
	IP_PKT = IP_putSrcNwk
				(
					IP_PKT,
					CNunit->Network,
					FALSE
				);
	
	IP_PKT = IP_putSourceInterface
				(
					IP_PKT,
					CNunit->Interface,
					FALSE
				);
	
	IP_PKT = IP_putDestinationInterface
				(
					IP_PKT,
					DestinationInterface,
					FALSE
				);
	
	IP_PKT = IP_putDestNwk
				(
					IP_PKT,
					DestinationNwk,
					FALSE
				);
	return (IP_PKT);	
}

/*-----------------------------------------
	Will dispatch the CN's packet 
 ------------------------------------------*/
int CNUnit_DispatchPacketWait
	(
		int RoutersPortNumber,
		PACKET_size_t PKT
	)
{
	MOBUNT_DispatchPacketWait
	(
		RoutersPortNumber,
		PKT
	);
	return 0;
}

/*-------------------------------------------------
 * This will bring up the CN's Interface
 * Ready for it to start listening on the
 * Socket 
 *------------------------------------------------*/
void* CNUnit_BringUpInterface
	(
		void* param	
	)
{
	CNunit->CNsListenPort = CNunit->Network * 1000 + 
							PORT_CLEAR + 
							CNunit->Interface;
	
	InitRouterChannel(CONNECT_CNUnit,CNUnit_Callback);

	printf
		(
		 "CNUnit listening on port %d \n",CNunit->CNsListenPort
		);

	InitServerSocket
		(
			CONNECT_CNUnit,
			"127.0.0.1",
			CNunit->CNsListenPort
		);
	
	while (TRUE)
	{
		WaitForEvents
			(
				CONNECT_CNUnit,
				WRITE_BACK_TO_SOCK_SERVER
			);
	}
}

/*------------------------------------------------------------------------------*
 * Parses the Input on the command line so that the CN can have 2 different     *
 * types of functionalities                                                     *
 * To listen on the socket and to                                               *
 * send on the socket                                                           *
 *------------------------------------------------------------------------------*/
int CN_main (int argc,char** argv)
{
	CNUnit_Init();
	
	/* nwk , intf, ranges usually 7000 */
	if (argc < 3)
	{
			printf("CN not given enough parameters\n");
			return (-1);
	}

	CNunit->Network      = atoi(argv[1]);
	CNunit->Interface    = atoi(argv[2]);
	CNunit->RouterRanges = atoi(argv[3]);

	return 0;
}
