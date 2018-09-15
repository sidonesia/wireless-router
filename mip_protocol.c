/**********************************************************************************************
 *                                                                                            *
 * This is the main implementation of the mip_protocol access functions                       *
 * All the functions act the same way. Bascially the following are the						  *
 * steps that are taken when wanting to check the packet formats.                             *
 *                                                                                            *
 *----------------------------------------------------------------------                      *
 * 1. MASK The packet so that we get only the section of the 32 bit                           *
 *    number we want.                                                                         *
 *                                                                                            *
 * 2. Shift this value ro the right so that the number now starts at the right hand side      *
 *    of the 32bit value                                                                      *
 *                                                                                            *
 * 3. Return this value                                                                       *
 *------------------------------------------------------------------------                    *
 * Below are the steps taken to set a value inside a packet                                   *
 *------------------------------------------------------------------------                    *
 * 1. shift the value we enter so that it is inline with the field in the                     *
 *    32 bit value                                                                            *
 *                                                                                            *
 * 2. do a logical or so that the values of zero are changed to that in                       *
 *    the value we enter.                                                                     *
 *                                                                                            *
 * 3. Return this value                                                                       *
 *-----------------------------------------------------****************************************
 * WRITTEN BY: Sidnei Budiman (2003)                   *
 *-----------------------------------------------------*
 */
#include "mip_protocol.h"

/*
	Gets the Packet Type 
 */
int MIPPROTO_getPacketType
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT_type = (packet & GET_PACKET_MASK);
	/* we bit shift it so that only the first 3 digits are left */
	PKT_type = PKT_type >> 29;
	return (PKT_type);
}

int MIPPROTO_putPacketType
    (
	    PACKET_size_t packet,
		PACKET_mask_t PKT_type

    )
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(29,PKT_type,packet);	
	return (PKT_new);
}

/*
	Looks at the packet and prints it out
	Maybe make this more verbose later one so that it
	is easier to debug
 */
int MIPPROTO_debugPacket
  (
               PACKET_size_t packet
  )
{
	int IX = 0;
	int BIT_m = INIT_BIT;
	for (IX = 31;IX >= 0 ;IX--)
		printf("%d",((packet & (BIT_m << IX))?(1):(0)));

	printf("\n");
	return 0;
}

PACKET_size_t MIPPROTO_unPackPacket
	(
		char* recvdPacket
	)
{
	return ((unsigned int)atoll(recvdPacket));
}

/* Dont forget to delete this packet */
int MIPPROTO_PackPacket
	(
		PACKET_size_t packet,
		char* PKTData
	)
{
	sprintf(PKTData,"%u",packet);
	return 0;
}

/*
	Will Set a series of Bits at a particular Location
	------------------------------------------------------	
	HOW IT DOES IT:
		Will use the bitwise OR operator to mask
		the bits into the packet
	------------------------------------------------------	
 */
unsigned int MIPPROTO_setBitAt
	(
		int position,
		unsigned int DataToSet,
		PACKET_size_t packet
	)
{
	PACKET_size_t DataToSetShifted = DataToSet << position;
	PACKET_size_t NewPacketData = packet | DataToSetShifted;	
	return NewPacketData;
}

/*-------- BELOW HERE ARE THE IP PACKET FUNCTIONS ---------*/

int IP_checkSrcNwk
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	)
{
	PACKET_size_t PKT;
	PACKET_size_t NEW_MASK;

	if(IS_TUNNELED)	
	{
		NEW_MASK = (IP_SRC_NWK_MASK >> 14);
		PKT = (packet & NEW_MASK);
		PKT = (PKT >> 11);
	}
	else
	{
		PKT = (packet & IP_SRC_NWK_MASK);
		PKT = (PKT >> 25);
	}
	return PKT;
}

int IP_putSrcNwk
	(
		PACKET_size_t packet,
		PACKET_mask_t SrcNwk,
		int DO_I_TUNNEL
	)
{
	PACKET_size_t PKT;
	if (DO_I_TUNNEL)
	{
		PKT = MIPPROTO_setBitAt(11,SrcNwk,packet);
	}
	else
	{
		PKT = MIPPROTO_setBitAt(25,SrcNwk,packet);
	}
	return PKT;
}

/*-----------------------------------------------------------*/

int IP_checkDestNwk
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	)
{
	PACKET_size_t PKT;
	PACKET_size_t NEW_MASK;

	if(IS_TUNNELED)
	{
		NEW_MASK = (IP_DES_NWK_MASK >> 14);
		PKT = (packet & NEW_MASK);
		PKT = (PKT >> 4);
	}
	else
	{
		PKT = (packet & IP_DES_NWK_MASK);
		PKT = (PKT >> 18);
	}
	return PKT;
}

int IP_putDestNwk
	(
		PACKET_size_t packet,
		PACKET_mask_t DestNwk,
		int DO_I_TUNNEL
	)
{
	PACKET_size_t PKT;
	if(DO_I_TUNNEL)
	{
		PKT = MIPPROTO_setBitAt(4,DestNwk,packet);
	}
	else
	{
		PKT = MIPPROTO_setBitAt(18,DestNwk,packet);
	}
	return PKT;
}


/*-----------------------------------------------------------*/

int IP_checkTunneledPkt
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT = (packet & IP_TUNNL_FLG_MASK);
	PKT = (PKT >> 28);
	return PKT;
}

int IP_putTunneledPkt
	(
		PACKET_size_t packet,
		PACKET_mask_t tunnel
	)
{

	PACKET_size_t PKT = MIPPROTO_setBitAt(28,tunnel,packet);
	return PKT;
}


/*-----------------------------------------------------------*/

int IP_checkSourceInterface
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	)
{
	PACKET_size_t PKT;
	PACKET_size_t NEW_MASK;

	if(IS_TUNNELED)
	{
		NEW_MASK = (IP_DES_NWK_MASK >> 14);
		PKT = (packet & NEW_MASK);
		PKT = (PKT >> 7);
	}
	else
	{
		PKT = (packet & IP_SRC_IF_MASK);
		PKT = (PKT >> 21);
	}
	return PKT;
}

int IP_putSourceInterface
	(
		PACKET_size_t packet,
		PACKET_mask_t src_address,
		int DO_I_TUNNEL
	)
{
	PACKET_size_t PKT;
	if(DO_I_TUNNEL)
	{
		PKT = MIPPROTO_setBitAt(7,src_address,packet);
	}
	else
	{
		PKT = MIPPROTO_setBitAt(21,src_address,packet);
	}
	return PKT;
}

/*-----------------------------------------------------------*/

int IP_checkDestinationInterface
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	)
{
	PACKET_size_t PKT;
	PACKET_size_t NEW_MASK;

	if(IS_TUNNELED)
	{
		NEW_MASK = (IP_DES_IF_MASK >> 14);
		PKT = (packet & NEW_MASK);
		PKT = (PKT >> 0);
	}
	else
	{
		PKT = (packet & IP_DES_IF_MASK);
		PKT = (PKT >> 14);
	}
	return PKT;
}


int IP_putDestinationInterface
	(
		PACKET_size_t packet,
		PACKET_mask_t des_address,
		int DO_I_TUNNEL
	)
{
	
	PACKET_size_t PKT;
	if(DO_I_TUNNEL)
	{
		PKT = MIPPROTO_setBitAt(0,des_address,packet);
	}
	else
	{
		PKT = MIPPROTO_setBitAt(14,des_address,packet);
	}
	return PKT;
}
/*-------- BELOW HERE ARE THE RIP PACKET FUNCTIONS ---------*/

/*
	Will check the check the source address 
	of where this packet originated from

 */
int RIP_checkSrcAddrROUTER
    (
	     PACKET_size_t packet
	)
{
    PACKET_size_t PKT = (packet & RIP_SRCADDR_MASK);
    PKT = (PKT >> 25);
	return (PKT);
}

/*
    Will put the source address of the current router

 */
int RIP_putSrcAddrROUTER
   (
	     PACKET_size_t packet,
		 PACKET_mask_t SRCAddr
   )
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(25,SRCAddr,packet);	
	return (PKT_new);
}

/*
    Will check the network ID of the rip packet
	to see which network is being advertised

 */
int RIP_checkNetworkID
    (
	     PACKET_size_t packet
	)
{
    PACKET_size_t PKT = (packet & RIP_NETADDR_MASK);
    PKT = (PKT >> 21);
	return (PKT);
}

/*
    Will put the network id of the network that wants to be
	advertised inside the packet 

 */
int RIP_putNetworkID
    (
	     PACKET_size_t packet,
		 PACKET_mask_t NetworkID
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(21,NetworkID,packet);	
	return (PKT_new);
}

/*
    Will check the distance to the advertised network
	this is the hop count cost 

 */
int RIP_checkDistanceToNwk
    (
	     PACKET_size_t packet
	)
{
    PACKET_size_t PKT = (packet & RIP_DISTADDR_MASK);
    PKT = (PKT >> 17);
	return (PKT);
		
}

/*
    Will put the distance into the packeat for the hotcount
	distance to the network being advertised

 */
int RIP_putDistanceToNwk
    (
	     PACKET_size_t packet,
		 PACKET_mask_t NetworkID
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(17,NetworkID,packet);	
	return (PKT_new);
}
/*-------- BELOW HERE ARE THE ICMP PACKET FUNCTIONS ---------*/


/* Test this against ICMP_AGT_HME && ICMP_AGT_FGN */
int ICMP_checkAgentType
    (
	    PACKET_size_t flag_segment
	)
{
	if ((flag_segment & ICMP_AGT_HME) == ICMP_AGT_HME)
		return (ICMP_AGT_HME);
	if ((flag_segment & ICMP_AGT_FGN) == ICMP_AGT_FGN)
		return (ICMP_AGT_FGN);
	return ERR;
}

/* will set the agent type */
PACKET_size_t ICMP_setAgentType
    (
	    PACKET_size_t flag_segment,
		PACKET_mask_t agentType
	)
{
    return (flag_segment | agentType);
}

int ICMP_checkDistNetworkRecv
	(
		PACKET_size_t packet
	)
{
	
	PACKET_size_t PKT_type = (packet & ICMP_DST_NET_MASK);
	PKT_type = PKT_type >> 0;
	return (PKT_type);
}

int ICMP_putDistNetworkRecv
	(
		PACKET_size_t packet,
		PACKET_mask_t Distance
	)
{
	
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(0,Distance,packet);	
	return (PKT_new);
}

/*
	Will check the Network which this packet came from

 */

int ICMP_checkRecvNetwork
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT_type = (packet & ICMP_RECV_NET_MASK);
	PKT_type = PKT_type >> 6;
	return (PKT_type);
}


int ICMP_putRecvNetwork
	(
		PACKET_size_t packet,
		PACKET_mask_t Network
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(6,Network,packet);	
	return (PKT_new);
}

/*
	Will check the ICMP_flags within the packet

 */
int ICMP_checkICMPFlags
	(	
		PACKET_size_t packet
	)
{
	// 000[101]00000000000000000000000000
	PACKET_size_t PKT_type = (packet & ICMP_FLAG_MASK);
	PKT_type = PKT_type >> 26; /* leave only the three bits that matter */
	return (PKT_type);
}

/*
 	Will set the value of the ICMP flags

 */

int ICMP_putICMPFlags
	(
		PACKET_size_t packet,
		PACKET_mask_t Flag
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(26,Flag,packet);	
	return (PKT_new);
}

int ICMP_checkLifeTime
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT_type = 0;

	PKT_type = (packet & ICMP_LIFETIME_MASK);
	PKT_type = PKT_type >> 18;
	return (PKT_type);
}

int ICMP_putLifeTime
	(
		PACKET_size_t packet,
		PACKET_mask_t LifeTime
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(18,LifeTime,packet);	
	return (PKT_new);
}
/*
	Will check the ICMP_Care OF Address Available within the
	Router.

 */
int ICMP_checkAvailCareOfAddress
	(
		PACKET_size_t packet,
		int COA_index
	)
{
	PACKET_size_t COA_MASK = 0;
	PACKET_size_t PKT_type = 0;
	int	      shiftRightByT = 0;

	assert (COA_index >=1 && COA_index <= 3);
	switch (COA_index)
	{
		case 1:
			COA_MASK = ICMP_1ST_COA_MASK;
			shiftRightByT = 22; /* the number to shift right by */
			break;
		case 2:
			COA_MASK = ICMP_2ND_COA_MASK;
			shiftRightByT = 18;
			break;
		case 3:
			COA_MASK = ICMP_3RD_COA_MASK;
			shiftRightByT = 14;
			break;
	}
	PKT_type = (packet & COA_MASK);
	PKT_type = PKT_type >> shiftRightByT;
	return (PKT_type);
}

/*
	Will set the Available care of address in the packet

 */
int ICMP_putAvailCareOfAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t CareOfAddress,
		int COA_index
	)
{
	PACKET_size_t PKT_new = 0;
	assert (COA_index >=1 && COA_index <= 3);

	
	switch (COA_index)
	{
		case 1:
			PKT_new = MIPPROTO_setBitAt(22,CareOfAddress,packet);
			break;
		case 2:
			PKT_new = MIPPROTO_setBitAt(18,CareOfAddress,packet);
			break;
		case 3:
			PKT_new = MIPPROTO_setBitAt(14,CareOfAddress,packet);
			break;
	}
	return (PKT_new);
}


/*
	Will check the ICMP SourceAddress of the Packet
	Probably wont be used but who knows, lets keep it

 */
int ICMP_checkSourceAddressOfPacket
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT_type = (packet & ICMP_SRC_ADDR_MASK);
	PKT_type = PKT_type >> 10;
	return (PKT_type);
}

/*
	Will set the source address of the packet

 */
int ICMP_putSourceAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t SourceAddressOfPacket
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(10,SourceAddressOfPacket,packet);
        return (PKT_new);
}

/*
	Will check the ICMP Destination Address of the Packet
	I can almost be sure this wont be used but lets keep it
	We got space in the packet anywayz.
 */
int ICMP_checkDestinationAddressOfPacket
	(
		PACKET_size_t packet
	)
{
	PACKET_size_t PKT_type = (packet & ICMP_DST_ADDR_MASK);
	PKT_type = PKT_type >> 6;
	return (PKT_type);
}

/*
	Will set the address field of the packet
	
 */
int ICMP_putDestinationAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t DestAddressOfPacket
	)
{
	PACKET_size_t PKT_new = MIPPROTO_setBitAt(6,DestAddressOfPacket,packet);
    return (PKT_new);
}

/*------------------ BELOW HERE ARE THE REQUEST PACKET FUNCTIONS --------------------*/
         /* All the functions below here are all related to the REQUEST Packet */
/*------------------ BELOW HERE ARE THE REQUEST PACKET FUNCTIONS --------------------*/

int REQT_checkREQTFlags
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_PROTO_FLAGS_MASK);
		PKT_type = PKT_type >> 27;
		return (PKT_type);
}

int REQT_putREQTFlags
    (
	      PACKET_size_t packet,
		  PACKET_mask_t Flags
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(27,Flags,packet);
		return (PKT_new);
}

/*
	Will set the lifeTime field of the packet
	
 */
int REQT_checkLifeTimeOfRegistration
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_PROTO_REG_MASK);
		PKT_type = PKT_type >> 23;
		return (PKT_type);
}

int REQT_putLifeTimeOfRegistration
    (
	     PACKET_size_t packet,
		 PACKET_mask_t LifeTime
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(23,LifeTime,packet);
		return (PKT_new);
}

/*
	Will set the Home address field of the packet
	
 */
int REQT_checkHomeAddress
    (
	     PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type =  (packet & REQT_HOME_ADDR_MASK);
		PKT_type = PKT_type >> 19;
		return (PKT_type);
}

int REQT_putHomeAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t HomeAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(19,HomeAddress,packet);
		return (PKT_new);
}
/*
	Will set the Home agent address field of the packet
	
 */
int REQT_checkHomeAgentAddress
    (
	     PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_HOME_AGNT_MASK);
		PKT_type = PKT_type >> 15;
		return (PKT_type);
}

int REQT_putHomeAgentAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t HomeAgentAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(15,HomeAgentAddress,packet);
		return (PKT_new);
}
/*
	Will set the care of address field of the packet
	
 */
int REQT_checkCareOfAddress
    (
	     PACKET_size_t packet
	)
{
        PACKET_size_t PKT_type = (packet & REQT_CARE_ADDR_MASK);
	    PKT_type = PKT_type >> 11;
	    return (PKT_type);
}

int REQT_putCareOfAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t CareOfAddress
	)
{
	    PACKET_size_t PKT_new = MIPPROTO_setBitAt(11,CareOfAddress,packet);	
		return (PKT_new);
}

/*
	Will set the identification field of the packet
	
 */
int REQT_checkIdentification
    (
	     PACKET_size_t packet 
	)
{
		PACKET_size_t PKT_type = (packet & REQT_ID_MASK);
		PKT_type = PKT_type >> 8;
		return (PKT_type);
}

int REQT_putIdentification
    (
	    PACKET_size_t packet,
		PACKET_mask_t Id
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(8,Id,packet);
		return (PKT_new);
}

/*
	Will set the source address field of the packet
	
 */
int REQT_checkSourceAddressOfPacket
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_SRC_ADDR_MASK);
		PKT_type = PKT_type >> 4;
		return (PKT_type);
}

int REQT_putSourceAddressOfPacket
    (
	     PACKET_size_t packet,
		 PACKET_mask_t SourceAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(4,SourceAddress,packet);
		return (PKT_new);
}

/*
	Will set the destination address field of the packet
	
 */
int REQT_checkDestinationAddressOfPacket
    (
        PACKET_size_t packet	 
	)
{
	    PACKET_size_t PKT_type = (packet & REQT_DST_ADDR_MASK);
		PKT_type = PKT_type >> 0;
		return (PKT_type);
}

int REQT_putDestinationAddressOfPacket
    (
	     PACKET_size_t packet,
		 PACKET_mask_t DestinationAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(0,DestinationAddress,packet);
		return (PKT_new);
}

/*--------------------- BELOW HERE ARE THE REPLY PACKET FUNCTIONS ------------------*/
         /* All the functions below here are all related to the REPLY Packet */
/*--------------------- BELOW HERE ARE THE REPLY PACKET FUNCTIONS ------------------*/


int REPLY_checkREQTFlags
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_PROTO_FLAGS_MASK);
		PKT_type = PKT_type >> 27;
		return (PKT_type);
}

int REPLY_putREQTFlags
    (
	      PACKET_size_t packet,
		  PACKET_mask_t Flags
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(27,Flags,packet);
		return (PKT_new);
}
/*
	Will set the lifeTime field of the packet
	
 */
int REPLY_checkLifeTimeOfRegistration
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_PROTO_REG_MASK);
		PKT_type = PKT_type >> 23;
		return (PKT_type);
}

int REPLY_putLifeTimeOfRegistration
    (
	     PACKET_size_t packet,
		 PACKET_mask_t LifeTime
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(23,LifeTime,packet);
		return (PKT_new);
}

/*
	Will set the Home address field of the packet
	
 */
int REPLY_checkHomeAddress
    (
	     PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type =  (packet & REQT_HOME_ADDR_MASK);
		PKT_type = PKT_type >> 19;
		return (PKT_type);
}


int REPLY_putHomeAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t HomeAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(19,HomeAddress,packet);
		return (PKT_new);
}
/*
	Will set the Home agent address field of the packet
	
 */
int REPLY_checkHomeAgentAddress
    (
	     PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_HOME_AGNT_MASK);
		PKT_type = PKT_type >> 15;
		return (PKT_type);
}

int REPLY_putHomeAgentAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t HomeAgentAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(15,HomeAgentAddress,packet);
		return (PKT_new);
}
/*
	Will set the care of address field of the packet
	
 */
int REPLY_checkCareOfAddress
    (
	     PACKET_size_t packet
	)
{
        PACKET_size_t PKT_type = (packet & REQT_CARE_ADDR_MASK);
	    PKT_type = PKT_type >> 11;
	    return (PKT_type);
}

int REPLY_putCareOfAddress
    (
	     PACKET_size_t packet,
		 PACKET_mask_t CareOfAddress
	)
{
	    PACKET_size_t PKT_new = MIPPROTO_setBitAt(11,CareOfAddress,packet);	
		return (PKT_new);
}
/*
	Will set the identification field of the packet
	
 */
int REPLY_checkIdentification
    (
	     PACKET_size_t packet 
	)
{
		PACKET_size_t PKT_type = (packet & REQT_ID_MASK);
		PKT_type = PKT_type >> 8;
		return (PKT_type);
}

int REPLY_putIdentification
    (
	    PACKET_size_t packet,
		PACKET_mask_t Id
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(8,Id,packet);
		return (PKT_new);
}
/*
	Will set the source address field of the packet
	
 */
int REPLY_checkSourceAddressOfPacket
    (
	    PACKET_size_t packet
	)
{
		PACKET_size_t PKT_type = (packet & REQT_SRC_ADDR_MASK);
		PKT_type = PKT_type >> 4;
		return (PKT_type);
}

int REPLY_putSourceAddressOfPacket
    (
	     PACKET_size_t packet,
		 PACKET_mask_t SourceAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(4,SourceAddress,packet);
		return (PKT_new);
}
/*
	Will set the destination address field of the packet
	
 */
int REPLY_checkDestinationAddressOfPacket
    (
        PACKET_size_t packet	 
	)
{
	    PACKET_size_t PKT_type = (packet & REQT_DST_ADDR_MASK);
		PKT_type = PKT_type >> 0;
		return (PKT_type);
}

int REPLY_putDestinationAddressOfPacket
    (
	     PACKET_size_t packet,
		 PACKET_mask_t DestinationAddress
	)
{
		PACKET_size_t PKT_new = MIPPROTO_setBitAt(0,DestinationAddress,packet);
		return (PKT_new);
}
