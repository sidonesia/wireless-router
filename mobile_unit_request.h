#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "table.h"
#include "mobile_unit.h"


/*
	This is the request packet sent to the HomeAgent when
	The Mobile unit registers with the home agent
 */
PACKET_size_t MUNITREQ_makeREQPKTSForHomeAgent(MobileUnit_t* MU);

/*
	This is the requst packet sent to the Foreign agent when
	the Mobile unit registers with the foreign agent
 */
PACKET_size_t MUNITREQ_makeREQPKTSForForeignAgentReg(MobileUnit_t* MU);
PACKET_size_t MUNITREQ_makeREQPKTSForForeignAgentDeReg(MobileUnit_t* MU);

