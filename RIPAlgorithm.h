#include "common_include.h"
#include "table.h"
#include "mip_protocol.h"
#include "GUIRouter.h"

/* 
	return whether or not the table was update if it wasnt then stop the
	thread that keeps sending out the 30 second packets
 */

typedef int NWK_ID; /* network id in the packet  */
typedef int RTR_ID; /* src router id in the packet   */
typedef int DST_NO; /* distance to router in pkt */


int RTALGO_updateNeighbourStatus
	(
		PACKET_size_t    PKT,
		RoutingTableOBJ* RT
	);

BOOLEAN RTALGO_updateTable
		(
			PACKET_size_t PKT,
			RoutingTableOBJ* RT,
			int ThisRouterId
		);

BOOLEAN RTALGO_addIntoRoutingTable
		(
			PACKET_size_t PKT,
			RoutingTableOBJ* RT
		);

BOOLEAN RTALGO_doesExist
		(
			PACKET_size_t PKT,
			RoutingTableOBJ* RT
		);

BOOLEAN RTALGO_isNextHopeFieldSame
		(
			PACKET_size_t PKT,
			RoutingTableOBJ* RT
		);

BOOLEAN RTALGO_isNewDistanceShorter
		(
			PACKET_size_t PKT,
			RoutingTableOBJ* RT
		);
