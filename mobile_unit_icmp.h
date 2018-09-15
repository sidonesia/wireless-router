#ifndef  _MU_ICMP_
#define  _MU_ICMP_

#include <pthread.h>
#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "table.h"
#include "mobile_unit.h"

int MUNIT_ICMP_setHomeAgent
	(
		MobileUnit_t* MU,
		PACKET_size_t HAaddress
	);

int MUNIT_ICMP_setFrgnAgent
	(
		MobileUnit_t* MU,
		PACKET_size_t FAaddress
	);

PACKET_size_t MUNIT_ICMP_buildICMPPkt
	(
		MobileUnit_t* MU
	);

#endif  
