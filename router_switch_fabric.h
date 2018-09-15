#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "table.h"

/*-----------------------------------
	will re-route RQST packets as 
	per the routing table parameter
 ------------------------------------*/
 // will return the next hope router to dispatch
 // the current RQST pkt to
int ROUTER_SWFC_rerouteRQSTPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t RQST_PKT
		);

/*-----------------------------------
	will re-route REPLY packets as 
	per the routing table parameter
 ------------------------------------*/
 // will return the next hope router to dispatch
 // the current REPLY pkt to
int ROUTER_SWFC_rerouteREPLYPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t REPLY_PKT
		);

/*------------------------------------
	will re-route IP packets as
	per the routing table parameter
 -------------------------------------*/
 // will return the next hope router to dispatch
 // the current IP pkt to
int ROUTER_SWFC_rerouteIPPKT
		(
			RoutingTableOBJ* RT,
			PACKET_size_t IP_PKT
		);
