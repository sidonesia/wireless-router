/*---------------------------------------------------------------
 *	RIP ROUTING ALGORITHM
 *---------------------------------------------------------------
 *		WRITTEN BY: SIDNEI BUDIMAN 2003 NWK RT & SW
 *---------------------------------------------------------------
 *
 *		This C code is an implemenation of the R.I.P ALGORITHM.
 *		This code simple determines if a recived packet should
 *		be added into the routing table or not.
 *
 *		If the following criteria described below is satisfied
 *		the pakt is added to the table
 *		
 *		IN THIS CODE 12 is regarded as infinity or the biggest
 *		possible number of hops
 *----------------------------------------------------------------*/
#include "RIPAlgorithm.h"


/*************************************************************
	below is the RIP algorithm implementation. This function 
	called from the main router code everytime a rip message
	is recieved.

	If any of the rules below evaluate to true then the 
	routing table gets updated:

	----------------------------------
	- RTALGO_doesExist
	----------------------------------
		* checks to see if the entry exists in the routing
		  table. If it doesnt then add it.
	----------------------------------
	- RTALGO_isNextHopeFieldSame
	----------------------------------
		* checks to see if the recived nxt hop is the same
		  as whats currently getting pointed to. If it is
		  then it will update the value
	----------------------------------
	- RTALGO_isNewDistanceShorter
	----------------------------------
		* checks to see if the recived distance to the
		  network is shorter then what is currently in the
		  routing table.
 *************************************************************/
BOOLEAN RTALGO_updateTable
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT,
		int ThisRouterId
	)
{
#ifdef DEBUG_ME
	RTR_ID R_ID = RIP_checkSrcAddrROUTER(PKT);
	NWK_ID N_ID = RIP_checkNetworkID(PKT);
	DST_NO D_NO = RIP_checkDistanceToNwk(PKT);
#endif
	
	/*
	 * if a router comes up then update its entry in the
	 * routing table from old value to 0 because now its
	 * back, we just recived a packet from it
	 */
	RTALGO_updateNeighbourStatus(PKT,RT);

#ifdef DEBUG_ME
	printf("PKT ::: PKT BINARY = ");
	MIPPROTO_debugPacket(PKT);

	printf("PKT ::: RTR_ID = %d\n",R_ID);
	printf("PKT ::: NWK_ID = %d\n",N_ID);
	printf("PKT ::: DST_NO = %d\n",D_NO);
#endif
	
	/*
	 * please add this entry if the entry does not exist in the routing table
	 * and the advertised network isnt the current networks id
	 */
	if (
		RTALGO_doesExist(PKT,RT) == FALSE 
		&& RIP_checkNetworkID(PKT) != ThisRouterId
	   )
	{
		RTALGO_addIntoRoutingTable(PKT,RT);
		addToMSGList(NULL,RT_NOT_EXIST,PKT);
		return (TRUE);
	}

	/* 
	 * please add this network if the next hop field
	 * is the same as whats current if the value of dist is not the same
	 */
	if (RTALGO_isNextHopeFieldSame(PKT,RT) == TRUE)
	{
			RTALGO_addIntoRoutingTable(PKT,RT);
			addToMSGList(NULL,RT_HOP_SAME_CHANGE,PKT);
			return (TRUE);
	}
	
	/*
	 * please update the table if the new distance is shorter then
	 * the current distance
	 */
	if (RTALGO_isNewDistanceShorter(PKT,RT) == TRUE)
	{
		RTALGO_addIntoRoutingTable(PKT,RT);
		addToMSGList(NULL,RT_HOP_SHORTER,PKT);
		return (TRUE);
	}
	return (FALSE);
}

/**************************************************************

	if a router accepts a rip packet we must automatically
	update this tables entry to zero because if it recives
	a packet it means that This router is awake and is a 
	direct neighbour.
	
 ***************************************************************/
int RTALGO_updateNeighbourStatus
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	)
{
	RTR_ID R_ID = RIP_checkSrcAddrROUTER(PKT);

	RT->setDestination(RT,R_ID);
	RT->setDistance(RT,R_ID,0);
	RT->setNextHopRoute(RT,R_ID,R_ID);
	return 0;
}

/***************************************************************

   function will simply read the data packet and then 
   add the pkt into the routing table

 ***************************************************************/
BOOLEAN RTALGO_addIntoRoutingTable
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	)
{
	RTR_ID R_ID = RIP_checkSrcAddrROUTER(PKT);
	NWK_ID N_ID = RIP_checkNetworkID(PKT);
	DST_NO D_NO = RIP_checkDistanceToNwk(PKT);
	
	RT->setDestination(RT,N_ID);
	/* Make sure that once its max it doesnt keep getting incremented */
	if(D_NO >= 12) RT->setDistance(RT,N_ID,12); 
	else
		RT->setDistance(RT,N_ID,D_NO+1);

	RT->setNextHopRoute(RT,N_ID,R_ID);
	return (TRUE);
}

/***************************************************************

	Does a simple check. Does the network exist in the
	routing table. if NO proceed to add it

 ***************************************************************/
BOOLEAN RTALGO_doesExist
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	)
{

	NWK_ID adv_nwk 			= RIP_checkNetworkID(PKT);
	RoutingTable RTInternal = findNetworkDestination(RT,adv_nwk);

	assert(PKT != 0 && RT != NULL);

	if (RTInternal.HasFieldBeenTaken == TRUE)
		return (TRUE);
	else
		return (FALSE);
}

/***************************************************************

	Does a simple check to see if the next hope field currently
	is the same as the one advertised.

	if it is then it proceeds to check if the distance value
	has changed, because if it hasnt then theres not need to
	update the value.

 ***************************************************************/
BOOLEAN RTALGO_isNextHopeFieldSame
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	)
{
	NWK_ID adv_nwk           = RIP_checkNetworkID(PKT);
	RTR_ID new_next_hop      = RIP_checkSrcAddrROUTER(PKT);
	RTR_ID current_next_hop  = RT->getNextHopRoute(RT,adv_nwk);

	int    new_dist = RIP_checkDistanceToNwk(PKT);
	int    old_dist = RT->getDistance(RT,adv_nwk);

	if((new_next_hop == current_next_hop) && (new_dist+1 != old_dist))
		return (TRUE);
	else
		return (FALSE);

}

/***************************************************************
	
	Does a simple check to see if the newly recieved pkt
	advertises a shorter path to the network, if it does
	then add it into the table
	
 ****************************************************************/
BOOLEAN RTALGO_isNewDistanceShorter
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	)
{
	NWK_ID adv_nwk     = RIP_checkNetworkID(PKT);
	DST_NO new_nwk_dst = RIP_checkDistanceToNwk(PKT);
	DST_NO old_nwk_dst = RT->getDistance(RT,adv_nwk);
	DST_NO new_dist_plus_one = new_nwk_dst + 1;

	if (new_dist_plus_one < old_nwk_dst)
		return TRUE;
	else
		return FALSE;
}
