/*								
 ################################################################
 # MIP_PROTOCOL module defines all the of the packet structures 
 # and protocols that will be sent in the application. 		
 ---------------------------------------------------------------#
 # 								
 # This file defines the					
 # ICMP Protocol (Modified Version):				
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 # 	That will be used as the router Advertisment packet	
 ---------------------------------------------------------------#
 # REQUEST Protocol (Modified Version):				
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 #	That will be used to request the routers to update	
 #	The tables and to advertise itself as an agent		
 ---------------------------------------------------------------#
 # REPLY Protocol (Modified Version):				
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 # 	That will be used to reply back from the Mobile Unit	
 #	To the routers and agents about its down details	
 ----------------------------------------------------------------
 # IP Protocol Packets (Modified Version)
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 #  The IP Protocol packet will allow the ack and send to be used
 #  between mobile units and remote hosts
 ----------------------------------------------------------------
 #	WRITTEN BY : SIDNEI BUDIMAN				
 ################################################################
 */

#ifndef MIP_PROTOCOL_H
#define MIP_PROTOCOL_H

#include "common_include.h"

/*
	As a general standard in my code
	a put function is a set function
	a check function is a read function
 */
#define INIT_BIT    0x1 	  
#define TRIGGER_PKT 0x0

typedef unsigned int     PACKET_size_t;  
/* define this here so if i decide to change it its easy */
typedef unsigned int     PACKET_mask_t; 
typedef char*            PACKET_send_t;
typedef struct
{
	long 	      address_value_int;  
	char* 	      address_value_char;
} AddressTranslator;

/*-----------------------------------*/
/* BELOW ARE IP  SPECIFIC FUNCTIONS  */
/*-----------------------------------*/
/*
 * [3bits] (type of packet)
 * [1bit ] Is Tunneled
 * [3bits] SrcNwk  from HA
 * [4bits] SrcIf   from HA 
 * [3bits] DestNwk from HA
 * [4bits] DestIf  from HA
 
 This is all the tunneled data 
 
 * [3bits] SrcNwk  from CN
 * [4bits] SrcIf   from CN
 * [3bits] DestNwk from CN
 * [4bits] DestIf  from CN
 */

/* AN IP PACKET TYPE [111] */

#define IP_PKT_TYPE       0x7
#define IP_TUNNL_FLG_MASK 0x10000000
#define IP_SRC_NWK_MASK   0xE000000
#define IP_SRC_IF_MASK    0x1E00000
#define IP_DES_NWK_MASK   0x1C0000 
#define IP_DES_IF_MASK    0x3C000
/* Do the payload later on */

/*-----------------------------------------------------------*/
int IP_checkSrcNwk
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	);

int IP_putSrcNwk
	(
		PACKET_size_t packet,
		PACKET_mask_t SrcNwk,
		int DO_I_TUNNEL
	);
/*-----------------------------------------------------------*/

int IP_checkDestNwk
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	);

int IP_putDestNwk
	(
		PACKET_size_t packet,
		PACKET_mask_t DestNwk,
		int DO_I_TUNNEL
	);
/*-----------------------------------------------------------*/

int IP_checkTunneledPkt
	(
		PACKET_size_t packet
	);

int IP_putTunneledPkt
	(
		PACKET_size_t packet,
		PACKET_mask_t tunnel
	);
/*-----------------------------------------------------------*/
int IP_checkSourceInterface
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	);

int IP_putSourceInterface
	(
		PACKET_size_t packet,
		PACKET_mask_t src_address,
		int DO_I_TUNNEL
	);

/*-----------------------------------------------------------*/
int IP_checkDestinationInterface
	(
		PACKET_size_t packet,
		int IS_TUNNELED
	);

int IP_putDestinationInterface
	(
		PACKET_size_t packet,
		PACKET_mask_t des_address,
		int DO_I_TUNNEL
	);


/*-----------------------------------*/
/* BELOW ARE RIP  SPECIFIC FUNCTIONS */
/*-----------------------------------*/
/*
 * Assume that this rip packet only has
 * - [3bits] (Type of packet) this is the same for all packets
 * - [4bits] The Source Router Address
 * - [4bits] The Network ID that it knows about
 * - [4bits] The Distance To that network
 */

/* A RIP PACKET TYPE [011] */
#define RIP_PKT_TYPE 0x3


/* Functions to get the chunks inside the packet */
#define RIP_SRCADDR_MASK   0x1E000000
#define RIP_NETADDR_MASK   0x1E00000
#define RIP_DISTADDR_MASK  0x1E0000


int RIP_checkSrcAddrROUTER
	(
		PACKET_size_t packet
	);

int RIP_putSrcAddrROUTER
	(
		PACKET_size_t packet,
		PACKET_mask_t SRCAddr
	);

int RIP_checkNetworkID
	(
		PACKET_size_t packet
	);

int RIP_putNetworkID
	(
		PACKET_size_t packet,
		PACKET_mask_t NetworkID
	);

int RIP_checkDistanceToNwk
	(
		PACKET_size_t packet
	);

int RIP_putDistanceToNwk
	(
		PACKET_size_t packet,
		PACKET_mask_t Distance
	);


int ADDRTRANS_covertAVI_to_AVC();
int ADDRTRANS_setAVI();
/*-----------------------------------*/
/* BELOW ARE ICMP SPECIFIC FUNCTIONS */
/*-----------------------------------*/
/*
	-----------------------------------------------
	NOTE:
	-----------------------------------------------
		Assume the router can only broadcast only 3 free care-of-address
		if one is taken it renews the taken one with a new one and then
		disperses this on the next ICMP packet.	

	MY VERSION OF AN ICMP PKT
	1) 3bits (Type of packet)
	2) 3bits (Certain Types Of Flags)
	3) 4bits (1st care-of-address that can be assigned) The forth bit shows its availability
	4) 4bits (2nd care-of-address that can be assigned) The forth bit shows its availability
	5) 4bits (3nd care-of-address that can be assigned) The forth bit shows its availability
	6) 4bits (Source Address of this packet)
	7) 4bits (Destination Address of this packet)

	// just recently added

	8) 3bits (recived Network address)
	9) 3bits (Distance of this network from recieved network address)

	-------------------------------------------------------
	ALL UP THIS PACKET USES 4+4+4+4+4+3+3+3+3 = (32 bits)
	-------------------------------------------------------

	For an ICMP PACKET for the left over bits we might need to
	use them to store the distances of the vectors for the next
	router, just some ints, we have 6 bits left so that can
	represent maximum link of 32 distance which should be
	enough.
*/

/* AN ICMP PACKET [001] */
#define ICMP_PKT_TYPE 0x1

#define ICMP_REG_REQ  0x0
#define ICMP_AGTBUSY  0x1
#define ICMP_AGT_HME  0x2
#define ICMP_AGT_FGN  0x4
#define ICMP_COA_AVL  0x1

/* BIT MASK VALUES TO GET THE CHUNKS INSIDE THE ICMP PACKET */

#define ICMP_FLAG_MASK      0x1C000000
#define ICMP_1ST_COA_MASK   0x3C00000
#define ICMP_LIFETIME_MASK  0x3C0000 /* replaced COA2 with lifetime */
#define ICMP_2ND_COA_MASK   0x3C0000 /* this should not be used  */
#define ICMP_3RD_COA_MASK   0x3C000
#define ICMP_SRC_ADDR_MASK  0x3C00
#define ICMP_DST_ADDR_MASK  0x3C0
#define ICMP_RECV_NET_MASK  0x38
#define ICMP_DST_NET_MASK   0x7

#define ICMP_COA_1 1
#define ICMP_COA_2 2
#define ICMP_COA_3 3

int ICMP_checkAgentType
    (
	    PACKET_size_t flag_segment
	);

PACKET_size_t ICMP_setAgentType
    (
	    PACKET_size_t flag_segment,
		PACKET_mask_t agentType
	);

/*-----------------------------------------------------------*/

/* the router this packet came from */
int ICMP_checkDistNetworkRecv
	(
		PACKET_size_t packet
	);

int ICMP_putDistNetworkRecv
	(
		PACKET_size_t packet,
		PACKET_mask_t Distance
	);

/*-----------------------------------------------------------*/

int ICMP_checkRecvNetwork
	(
		PACKET_size_t packet
	);

int ICMP_putRecvNetwork
	(
		PACKET_size_t packet,
		PACKET_mask_t Network
	);

/*-----------------------------------------------------------*/
int ICMP_checkICMPFlags
	(
		PACKET_size_t packet
	);

int ICMP_putICMPFlags
	(
		PACKET_size_t packet, 
		PACKET_mask_t Flag
	);

/*-----------------------------------------------------------*/


int ICMP_checkLifeTime
	(
		PACKET_size_t packet
	);

int ICMP_putLifeTime
	(
		PACKET_size_t packet, 
		PACKET_mask_t LifeTime
	);
/*-----------------------------------------------------------*/
int ICMP_checkAvailCareOfAddress
	(
		PACKET_size_t packet,
		int COA_index
	);

int ICMP_putAvailCareOfAddress
	(
		PACKET_size_t packet, 
		PACKET_mask_t CareOfAddress,
		int COA_index
	);

/*-----------------------------------------------------------*/
int ICMP_checkSourceAddressOfPacket
	(
		PACKET_size_t packet
	);

int ICMP_putSourceAddressOfPacket
	(
		PACKET_size_t packet, 
		PACKET_mask_t SourceAddressOfPacket
	);

/*-----------------------------------------------------------*/
int ICMP_checkDestinationAddressOfPacket
	(
		PACKET_size_t packet
	);

int ICMP_putDestinationAddressOfPacket
	(
		PACKET_size_t packet, 
		PACKET_mask_t DestAddressOfPacket
	);
/*-----------------------------------------------------------*/

/*---------------------------------------------------------*/
/* BELOW ARE THE PACKET DEFINITIONS FOR THE REQUEST PACKET */
/*---------------------------------------------------------*/

/*
	MY VERSION OF A REQUEST CARE-OF-ADDRESS PKT
	1) 3bits (Type of packet)
	2) 2bits (Type of flags)
	3) 4bits (Registertion valid time)
	4) 4bits (Home Address of the Mobile Unit)
	5) 4bits (Home Agent Address of the Mobile Unit)
	6) 4bits (Care of Address of the Mobile Unit)
	7) 3bits (Identification of the Mobile Unit)
	8) 4bits (Source Address of this packet)
	9) 4bits (Destination Address of this packet)
	-------------------------------------------------------
	ALL UP THIS PACKET USES 3+2+4+4+4+4+3+4+4 = (32 bits)
	-------------------------------------------------------

 */

/* A REQUEST PACKET [010] */
#define  REQT_PKT_TYPE 0x2 //010
#define  REQT_RETAIN_PRIOR_CAREADDR 0x1
#define  REQT_TUNNEL_BCST_MESG	    0x2

/* used flags */
#define  REQT_PKT_DEREGISTER        0x0
#define  REQT_PKT_DIRECT_FROM_MU    0x1
#define  REQT_PKT_IS_A_RELAY        0x3

/* START REQUIRED MASKS FOR THE PACKET */

#define  REQT_PROTO_FLAGS_MASK 0x18000000 
#define  REQT_PROTO_REG_MASK   0x7800000
#define  REQT_HOME_ADDR_MASK   0x780000
#define  REQT_HOME_AGNT_MASK   0x78000
#define  REQT_CARE_ADDR_MASK   0x7800
#define  REQT_ID_MASK          0x700
#define  REQT_SRC_ADDR_MASK    0xF0
#define  REQT_DST_ADDR_MASK    0xF

/* END  REQUIRED MASKS FOR THE PACKET */

/*-----------------------------------------------------------*/
int REQT_checkREQTFlags
	(
		PACKET_size_t packet
	);

int REQT_putREQTFlags
	(
		PACKET_size_t packet,
		PACKET_mask_t Flags
	);

/*-----------------------------------------------------------*/
int REQT_checkLifeTimeOfRegistration
	(
		PACKET_size_t packet
	);

int REQT_putLifeTimeOfRegistration
	(
		PACKET_size_t packet,
		PACKET_mask_t LifeTime
	);

/*-----------------------------------------------------------*/
int REQT_checkHomeAddress
	(
		PACKET_size_t packet
	);

int REQT_putHomeAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t HomeAddress
	);

/*-----------------------------------------------------------*/
int REQT_checkHomeAgentAddress
	(
		PACKET_size_t packet
	);

int REQT_putHomeAgentAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t HomeAgentAddress
	);

/*-----------------------------------------------------------*/
int REQT_checkCareOfAddress
	(
		PACKET_size_t packet
	);

int REQT_putCareOfAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t CareOfAddress
	);

/*-----------------------------------------------------------*/
int REQT_checkIdentification
	(
		PACKET_size_t packet
	);

int REQT_putIdentification
	(
		PACKET_size_t packet,
		PACKET_mask_t Id
	);

/*-----------------------------------------------------------*/
int REQT_checkSourceAddressOfPacket
	(
		PACKET_size_t packet
	);

int REQT_putSourceAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t SourceAddress
	);

/*-----------------------------------------------------------*/
int REQT_checkDestinationAddressOfPacket
	(
		PACKET_size_t packet
	);

int REQT_putDestinationAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t DestinationAddress
	);
/*-----------------------------------------------------------*/

/***********************************************/
/* BELOW ARE THE REPLY PACKET FUNCTION SUPPORT */
/***********************************************/

/*
	MY VERSION OF A REPLY CARE-OF-ADDRESS PKT
	1) 3bits (Type of packet) 
	2) 2bits (Type of flags)
	3) 4bits (Registertion valid time)
	4) 4bits (Home Address of the Mobile Unit)
	5) 4bits (Home Agent Address of the Mobile Unit)
	6) 4bits (Care of Address of the Mobile Unit)
	7) 3bits (Identification of the Mobile Unit)
	8) 4bits (Source Address of this packet)
	9) 4bits (Destination Address of this packet)

	-------------------------------------------------------
	ALL UP THIS PACKET USES 3+2+4+4+4+4+3+4+4 = (32 bits)
	-------------------------------------------------------
 */

/* A REPLY PACKET [100] */
#define  REPLY_PKT_TYPE 0x4
#define  REPLY_RETAIN_PRIOR_CAREADDR 0x1
#define  REPLY_TUNNEL_BCST_MESG      0x2

/* START REQUIRED MASKS FOR THE PACKET */

#define  REPLY_PROTO_FLAGS_MASK 0x18000000 
#define  REPLY_PROTO_REG_MASK   0x7800000
#define  REPLY_HOME_ADDR_MASK   0x780000
#define  REPLY_HOME_AGNT_MASK   0x78000
#define  REPLY_CARE_ADDR_MASK   0x7800
#define  REPLY_ID_MASK          0x700
#define  REPLY_SRC_ADDR_MASK    0xF0
#define  REPLY_DST_ADDR_MASK    0xF

/* END  REQUIRED MASKS FOR THE PACKET */

/*-----------------------------------------------------------*/
int REPLY_checkREPLYFlags
	(
		PACKET_size_t packet
	);

int REPLY_putREPLYFlags
	(
		PACKET_size_t packet,
		PACKET_mask_t Flags
	);

/*-----------------------------------------------------------*/
int REPLY_checkLifeTimeOfRegistration
	(
		PACKET_size_t packet
	);

int REPLY_putLifeTimeOfRegistration
	(
		PACKET_size_t packet,
		PACKET_mask_t LifeTime
	);

/*-----------------------------------------------------------*/
int REPLY_checkHomeAddress
	(
		PACKET_size_t packet
	);

int REPLY_putHomeAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t HomeAddress
	);

/*-----------------------------------------------------------*/
int REPLY_checkHomeAgentAddress
	(
		PACKET_size_t packet
	);

int REPLY_putHomeAgentAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t HomeAgent
	);

/*-----------------------------------------------------------*/
int REPLY_checkCareOfAddress
	(
		PACKET_size_t packet
	);

int REPLY_putCareOfAddress
	(
		PACKET_size_t packet,
		PACKET_mask_t CareOfAddress
	);

/*-----------------------------------------------------------*/
int REPLY_checkIdentification
	(
		PACKET_size_t packet
	);

int REPLY_putIdentification
	(
		PACKET_size_t packet,
		PACKET_mask_t Id
	);

/*-----------------------------------------------------------*/
int REPLY_checkSourceAddressOfPacket
	(
		PACKET_size_t packet
	);

int REPLY_putSourceAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t SourceAddress
	);

/*-----------------------------------------------------------*/
int REPLY_checkDestinationAddressOfPacket
	(
		PACKET_size_t packet
	);

int REPLY_putDestinationAddressOfPacket
	(
		PACKET_size_t packet,
		PACKET_mask_t DestinationAddress
	);
/*-----------------------------------------------------------*/

/********************************************/
/* BELOW ARE THE GENERIC PROTOCOL FUNCTIONS */
/********************************************/

#define GET_PACKET_MASK 0xE0000000

int MIPPROTO_getPacketType
	(
		PACKET_size_t packet
	);

int MIPPROTO_putPacketType
	(
		PACKET_size_t packet,
		PACKET_mask_t PKT_Type
	);

int MIPPROTO_debugPacket
	(
		PACKET_size_t packet
	);

int MIPPROTO_getPacketRange
	(
		int start,
		int end,
		PACKET_size_t packet
	);

unsigned int MIPPROTO_setBitAt
	(
		int position,
		unsigned int DataToSet,
		PACKET_size_t packet
	); 

PACKET_size_t MIPPROTO_unPackPacket
	(
		char* recvdPackt 
	);

int MIPPROTO_PackPacket
	(
		PACKET_size_t packet,
		char* PKTData
	);

#endif 
