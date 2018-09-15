/*
 * Here we have socket wrapper functions
 * can mask away all the socket calls into
 * this module. This file siply justs
 * sets up all the requred parameters for creating
 * a sokcet to send data on and a socket to listen on
 */

#ifndef  ROUTER_SOCKET_CHANNLE_H
#define  ROUTER_SOCKET_CHANNLE_H

#define  MAX_Q_SIZE     50
#define  MAX_NEIGHBOURS 10

/*
   when a connection is recived we can either choose to write it back
   directly on the socket that was recieved or choose to write it back
   to the other routers server socket.

   We use these flags depending on the situation
 */
#define  WRITE_BACK_TO_SOCK_DIRECT 1
#define  WRITE_BACK_TO_SOCK_SERVER 0

#define  DISPATCH_PACKET_READER    1
#define  DISPATCH_PACKET_WRITER    0

#define  BIND_ERR                 -2
typedef  int        SOCKET;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>

#include "common_include.h"
#include "mip_protocol.h"
/*
 * For every server socket we have a collection
 * of client sockets as well
 */
typedef struct
{
	int MAX_ARRAY_SIZE;

	SOCKET ServerSocket;
	SOCKET DirectClientSocket;
	SOCKET ClientSocket[MAX_NEIGHBOURS];

	struct sockaddr_in remote_ProcAddress;
	int    portNum;
	int    sin_struct_size;
	int    (*CallbackActionFunc)(char*);

} RouterCONN_Channel;

RouterCONN_Channel* new_RCChan();

/*
 * will initialise the client socket to 0
 */
int InitRouterChannel
	(
		RouterCONN_Channel* RCChan,
		//int (*CallbackActionFunc)(PACKET_mask_t*)
		int (*CallbackActionFunc)(char*)
	);

int InitServerSocket
	(
	    RouterCONN_Channel* RCChan,	
		char*               Host,
		int 				PORT_LISTEN_ON
	);

int DispatchPacket
	(
	    int           OPT,
		int           RemotePort,
		char*         RemoteHostName,
		char*         PACKET
	);

int WaitForEvents
	(
		RouterCONN_Channel* RCChan,
		int   WriteBackOption
	);
#endif 

