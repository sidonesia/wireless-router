#include "router_socket_channel.h"

RouterCONN_Channel* new_RCChan()
{
	RouterCONN_Channel* RC = malloc(sizeof (RouterCONN_Channel));
	RC->MAX_ARRAY_SIZE = MAX_NEIGHBOURS;
	return (RC);
}


/*----------------------------------------------------------------------------*/
/* will set up the required ports and the callback function to be invoked  when a pkt is revcd*/
int InitRouterChannel
	(
		RouterCONN_Channel* RCChan,
		//int (*CallbackActionFunc)(PACKET_mask_t*)
		int (*CallbackActionFunc)(char*)
	)
{
	int ix = 0;
	for (ix = 0; ix < RCChan->MAX_ARRAY_SIZE; ix ++)
	{
		RCChan->ClientSocket[ix] = ERR; /* init them all to -1 value */
	}
	RCChan->ServerSocket       = ERR;
	RCChan->DirectClientSocket = ERR;

	/* Registering our callback functions */
	RCChan->CallbackActionFunc = CallbackActionFunc;
	return 0;
}

/*----------------------------------------------------------------------------*/
/* will start the initialisation and creation of the sockets*/
int InitServerSocket
	(
		RouterCONN_Channel* RCChan,
		char*    			Host,
		int                 PORT_LISTEN_ON
	)
{
    struct sockaddr_in local_ProcAddress;
	int    OPT = TRUE;
	
	RCChan->portNum = PORT_LISTEN_ON;
	if (
		(RCChan->ServerSocket = socket(AF_INET,SOCK_STREAM,0)) == ERR 
	   )
	{
		printf("ServerSocket Creation Error \n");
		exit (ERR);
	}
	

	if (
		 setsockopt ( RCChan->ServerSocket,SOL_SOCKET,SO_REUSEADDR,&OPT,sizeof(int)) == ERR 
	   )
	{
		printf("SetSockOpt Error\n");
		exit (ERR);
	}

	local_ProcAddress.sin_family = AF_INET;
								 /* 
								    very important for byte ordering, if this is not added it
									wont work for an intel machine aritecture
								 */
	local_ProcAddress.sin_port = htons(PORT_LISTEN_ON);
	local_ProcAddress.sin_addr.s_addr = inet_addr(Host); /* this value muse be read from file*/
	memset(&(local_ProcAddress.sin_zero),'\0',8);

	if (
			bind(RCChan->ServerSocket,(struct sockaddr *)&local_ProcAddress,
										sizeof(struct sockaddr)) == ERR
	   )
	{
		printf("BIND ERROR ON PORT %d\n",PORT_LISTEN_ON);
		return (BIND_ERR);
		//exit (ERR);
	}

	if (listen(RCChan->ServerSocket,MAX_Q_SIZE) == ERR )
	{
		printf("listen Error \n");
		exit(ERR);
	}
	return 0;
}


/*----------------------------------------------------------------------------*/
/* will be the function that sits and waits for in comming packes */
int WaitForEvents
	(
		RouterCONN_Channel* RCChan,
		int   WriteBackOptions
	)
{
	BOOLEAN EXIT = FALSE;

	while (EXIT == FALSE)
	{
		if (
			(RCChan->DirectClientSocket = accept(RCChan->ServerSocket,
												(struct sockaddr *)&RCChan->remote_ProcAddress,
												&RCChan->sin_struct_size)) == ERR
	   	)
    	{
			printf("accept Error \n");
			close(RCChan->ServerSocket);

			InitServerSocket(RCChan,"127.0.0.1",RCChan->portNum);
			//exit(ERR);
		}
		EXIT = TRUE;
	}

	/*
	 *  We should write the callback function here 
	 */

	if (WriteBackOptions == WRITE_BACK_TO_SOCK_SERVER)
	{
		/* This is our 32 bit packet */
		//unsigned int RawPacket;
		char buffer[11];
		memset (buffer,'\0',11);
		/* 
		    The parameter that we pass into here should be the packet 
			This packet will then be decoded and the appropriate action
			be taken. Then this process will be a client and send the packet
			else where
		*/

		recv(RCChan->DirectClientSocket,buffer,sizeof(buffer),0);

		RCChan->CallbackActionFunc(buffer);

		/* Above is the callback function registerd to be called when a pakcet is recieved*/
	}
	else
	{
		// write straight to RCChan->DirectClientSocket FD
		send(RCChan->DirectClientSocket,"Test Connection",16,0);
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/* will be the function that sends the packet to another process defined by port and remote
   host name 
 */
int DispatchPacket
	(
	    int           OPT,
		int   		  RemotePort,
		char*         RemoteHostName, /* will usually be localhost for this application */
		char*         PACKET
	)
{
    SOCKET             SocketConnection;
	struct hostent  *  HostEntity;
	struct sockaddr_in remote_address;
	
	if ( (HostEntity = gethostbyname(RemoteHostName)) == NULL)
	{
		printf("gethostbyname error \n");
		return (ERR);
	}
	if ((SocketConnection = socket(AF_INET,SOCK_STREAM,0)) == ERR)
	{
		printf("socket error when dispatching to port %d\n",RemotePort);
		close(SocketConnection);
		return ERR;
	}
	remote_address.sin_family = AF_INET;
	remote_address.sin_port = htons(RemotePort);
	remote_address.sin_addr = *((struct in_addr *)HostEntity->h_addr);
	memset(&(remote_address.sin_zero),'\0',8);

	if(
			connect (
					  SocketConnection,(struct sockaddr *)&remote_address,
					  sizeof(struct sockaddr)
					) == ERR
	  )
	{
		close(SocketConnection);
		return ERR;
	}
	if (OPT == DISPATCH_PACKET_WRITER)
	{
#ifdef DEBUG_ME
		printf ("About To Write ");
		MIPPROTO_debugPacket( MIPPROTO_unPackPacket(PACKET));
#endif
#ifdef DEBUG_ME
		printf (" sizeof %d \n",sizeof (PACKET_mask_t));
#endif
		
		/* will dispatch this packet to the server socket of the router */
		if(send (SocketConnection,(char*)PACKET,strlen(PACKET),0) == ERR)
		{
			printf("send error \n");
			close(SocketConnection);
			exit(ERR);
		}
		//close (SocketConnection);
	}
	else
	{
		/* 
			here we will write the recieve function but we dont know if we
			really need it just yet so leave it out
		*/
		//close (SocketConnection);
	}
	close(SocketConnection);
	return 0;
}

/*----------------------------------------------------------------------------*/

