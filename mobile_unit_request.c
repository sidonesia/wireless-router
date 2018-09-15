#include "mobile_unit_request.h"


PACKET_size_t MUNITREQ_makeREQPKTSForHomeAgent
	(
		MobileUnit_t* MU
	)
{
	PACKET_size_t REQST_PKT = 0;
	REQST_PKT = MIPPROTO_putPacketType(REQST_PKT,REQT_PKT_TYPE);

	/*------------------------------------------------- 
	    send this back to the home agent so that
		it knows the ip address of the mobile unit
	 -------------------------------------------------*/
#ifdef DEBUG_ME
	printf("This is the homeaddress %d \n",
				MUnit->HAgentDetails.MobUnitHomeAgentPort);
#endif

	REQST_PKT = REQT_putHomeAddress
				(
					REQST_PKT,
					/* this is the random number between 1-15*/
					MUnit->HAgentDetails.MobUnitHomeAgentPort
				);
	
	/*-------------------------------------------------------
		send this back to the home agent so that 
		the HomeAddress MUnit->CurrentNetworkPort + 
		the homeagent network address * 1000 can define the
		whole port of the mobile unit
	 -------------------------------------------------------*/
	REQST_PKT = REQT_putHomeAgentAddress
				(
					REQST_PKT,
					MUnit->HAgentDetails.MobUnitHomeAgentNwk
				);
	return (REQST_PKT);
}

/*---------------------------------------------------------------------------*/

PACKET_size_t MUNITREQ_makeREQPKTSForForeignAgentDeReg
            (
			    MobileUnit_t* MU
			)
{
    PACKET_size_t REQST_PKT = 0;
	
	/* ofcourse we need to set the packet type */
	REQST_PKT = MIPPROTO_putPacketType
					(
						REQST_PKT,
						REQT_PKT_TYPE
					);
	/* 
	 * we need to put the care of address so we can tell the router which one
	 * to invalidate
	 */
	REQST_PKT = REQT_putCareOfAddress	
					(
						REQST_PKT,
						MU->CurrentNetworkPort
					);
	return REQST_PKT;
}

/*-------------------------------------------------------
	We set the rest of the fields in this packet in the
	actual Forign agent its self
 --------------------------------------------------------*/
PACKET_size_t MUNITREQ_makeREQPKTSForForeignAgentReg
			(
				MobileUnit_t* MU
			)
{
	PACKET_size_t REQST_PKT = 0;
	PACKET_size_t REQST_FLAG_SEGMENT_PKT = 0;
	
	REQST_FLAG_SEGMENT_PKT = 
			REQST_FLAG_SEGMENT_PKT | REQT_PKT_DIRECT_FROM_MU;

	REQST_PKT = REQT_putDestinationAddressOfPacket
					(
						REQST_PKT,
						MU->HAgentDetails.MobUnitHomeAgentNwk
					);
	
	REQST_PKT = REQT_putREQTFlags
					(
						REQST_PKT,
						REQST_FLAG_SEGMENT_PKT
					);
	
	/* set the packet type */
	REQST_PKT = MIPPROTO_putPacketType
					(
						REQST_PKT,
						REQT_PKT_TYPE
					);
	/* set the care of address it wishes to use in the packet */
	REQST_PKT = REQT_putCareOfAddress	
					(
						REQST_PKT,
						MU->CurrentNetworkPort
					);
	
	/* set the home address of this MU in the packet */
	REQST_PKT = REQT_putHomeAddress
					(
						REQST_PKT,
						MUnit->HAgentDetails.MobUnitHomeAgentPort
					);

	/* set the home agent of this MU in the packet */
	REQST_PKT = REQT_putHomeAgentAddress
					(
						REQST_PKT,
						MUnit->HAgentDetails.MobUnitHomeAgentNwk
					);
	
	/* 
	   set the amount of time this packet wishes to be registerd
	   in this foreign network
	 */
	REQST_PKT = REQT_putLifeTimeOfRegistration
					(
						REQST_PKT,
						MUnit->LifeTimeOfReg /* change this later so it is not HC */
					);
	return (REQST_PKT);
}
