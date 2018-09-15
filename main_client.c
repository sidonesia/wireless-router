#include "fileIO.h"
#include "mip_protocol.h"
#include "mobile_unit.h"
#include "router.h"
#include "router_socket_channel.h"
#include "table.h"
#include "common_include.h"

int main (int argc,char** argv)
{
	if (argc >= 2)
		MOBUNT_Init(argv[1]);
    return 0;
}
