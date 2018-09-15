#ifndef __MOBILE__UNIT__
#define __MOBILE__UNIT__

#include <pthread.h>
#include "common_include.h"
#include "router_socket_channel.h"
#include "mip_protocol.h"
#include "mobile_unit.h"
#include "table.h"
#include "GUICn.h"

#define  SEND_MODE 1
#define  RECV_MODE 0

/*--------------------------------------------------------------*
	We need the Router Range and the Network to know what 
	is the source address IE this CN unit

	We need to implement a function inthe router which 
	will send the IP packet to the host once the 
	packet has reached the correct router
 *--------------------------------------------------------------*/

typedef struct
{
		int Network;       /* the network this CN is on */
		int Interface;     /* the interface of this CN */
		int RouterRanges; 
		int CNsListenPort;
		int NetworkRouterPort; /* The router port for this network */

}
CN_Unit;

RouterCONN_Channel*  CONNECT_CNUnit;
CN_Unit*        	 CNunit;
int 			     DestinationNwk;
int 				 DestinationInterface;
int                  MODE;

/*-------------------------------------------------------*/
/* The function envoked when the CN recieves a ip packet */
/*-------------------------------------------------------*/
int CNUnit_Callback
       (
		   PACKET_send_t RawPacket
	   );

PACKET_size_t CNUnit_makeIPPKTs();

/*----------------------------------------------*/
/* The function the CN uses to dispatch packets */	   
/*----------------------------------------------*/
int CNUnit_DispatchPacketWait
	  (
	  	   int RoutersPortNumber,
		   PACKET_size_t PKT
	  );

/*--------------------------------------*/
/* initialise all of the CN's variables */
/*--------------------------------------*/
int CNUnit_Init();
int CN_main();
void* CNUnit_BringUpInterface
	(
		void* param
	);

#endif

