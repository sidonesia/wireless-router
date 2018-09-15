/*
 * This switching fabric is the actual file
 * that directly access the routing table with 
 * the network ID and then gets the actual 
 * Next hop jump value that is needed so that
 * The next route for the pkt is known
 */

#include "router_switch_fabric.h"

/* handles request packets and returns the next hop */
int  ROUTER_SWFC_rerouteRQSTPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t RQST_PKT
		)
{
	int NET_ID = REQT_checkDestinationAddressOfPacket(RQST_PKT);
	/* returns the next hop*/
	int NXT_HOP= RT->getNextHopRoute(RT,NET_ID);
#ifdef DEBUG_ME	
	printf("NETID in router fabric is %d \n",NET_ID);
#endif
	return (NXT_HOP);
}

/* handles reply pkts and returns the next hop */
int ROUTER_SWFC_rerouteREPLYPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t REPLY_PKT
		)
{
	int NET_ID = REPLY_checkDestinationAddressOfPacket(REPLY_PKT);
	/* returns the next hop*/
	int NXT_HOP= RT->getNextHopRoute(RT,NET_ID);
#ifdef DEBUG_ME	
	printf("NETID in router fabric is %d \n",NET_ID);
#endif
		return (NXT_HOP);
}

/* handles IP pkts and returns the next hop */
int ROUTER_SWFC_rerouteIPPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t IP_PKT
		)
{
	int IS_TUNNELED = IP_checkTunneledPkt(IP_PKT);
	int NET_ID 		= IP_checkDestNwk(IP_PKT,IS_TUNNELED);
	/* returns the next hop*/
	int NXT_HOP		= RT->getNextHopRoute(RT,NET_ID);;
#ifdef DEBUG_ME
	printf("NETID IP PKT in router fabric is %d \n",NET_ID);
#endif
		return (NXT_HOP);

}
