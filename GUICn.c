#include "GUICn.h"

int main(int argc,char** argv)
{
	if (argc < 3)
		exit -1;
	
	initStructuresCN();


	CN_main(argc,argv);
	startNetworkInterface ();
	createCnGui (argc,argv);	
	return 0;
}

int initStructuresCN()
{
	int ix = 0;

	GC.ListCounter = 0;
	for (ix =0;ix < 199;ix++)
	{
		MSGList[ix] = malloc(sizeof(char)*100);
	}
	MSGList[199] = NULL;

	return 0;
}

int startNetworkInterface ()
{
	pthread_t thandle;
	pthread_create(&thandle,NULL,startNetworkInterfaceWrapper,NULL);
	return 0;
}

void* startNetworkInterfaceWrapper(void* param)
{
	CNUnit_BringUpInterface(NULL);
	return NULL;
}

void   sendIpPacket(Control* C)
{
	pthread_t thandle;
	pthread_create(&thandle,NULL,sendIpPacketWrapper,NULL);
}

/*
  done forget to add ip packet information to the router lists
 */

int addToMsgList(char* msg,PACKET_size_t PKT)
{
	char TIMEBUFF[30];
	char SRCPKTBUFF[100];
	char IPBUFF[30];
	char IPBUFF2[30];
	int  SRCNwk = 0;
	int  SRCInf = 0;

	int  SRCCoANwk = 0;
	int  SRCCoAInf = 0;

	if (GC.ListCounter == 200)
		GC.ListCounter = 0;
	
	SRCNwk = IP_checkSrcNwk(PKT,IP_checkTunneledPkt(PKT));
	SRCInf = IP_checkSourceInterface(PKT,IP_checkTunneledPkt(PKT));

	SRCCoANwk = IP_checkSrcNwk(PKT,TRUE);
	SRCCoAInf = IP_checkSourceInterface(PKT,TRUE);
	
	getTimeNow(TIMEBUFF);
	convertPortToIpFormat(SRCNwk,SRCInf,IPBUFF);
	convertPortToIpFormat(SRCCoANwk,SRCCoAInf,IPBUFF2);

	sprintf(SRCPKTBUFF,"%s FROM SRC HmA : %s : SRC NWK: %s",msg,IPBUFF,IPBUFF2);
	/* have to say where the bloody packet came from */
	sprintf(MSGList[GC.ListCounter],"[%s]: %s",TIMEBUFF,SRCPKTBUFF);
	change_list_box(GC.MSGList,MSGList);
	redraw_control(GC.MSGList);
	GC.ListCounter++;

	return 0;
}


void* sendIpPacketWrapper(void* param)
{
	PACKET_size_t PKT; 
	char          IPBUFF[20];
	
	DestinationNwk       = atoi (get_control_text(GC.NwkText)); 
	DestinationInterface = atoi (get_control_text(GC.InfText));

	if (!(DestinationNwk && DestinationInterface))
		printf("NO FIELDS CAN BE EMPTY \n");
	
	CNunit->NetworkRouterPort = CNunit->RouterRanges +  CNunit->Network; 
	
	convertPortToIpFormat(DestinationNwk,DestinationInterface,IPBUFF);
	set_control_text(GC.IPText,IPBUFF);
	
	PKT = CNUnit_makeIPPKTs();
	CNUnit_DispatchPacketWait
		(
			CNunit->NetworkRouterPort,
			PKT
		);
	return NULL;
}

int createCnGui(int argc,char** argv)
{
	/* save code for config file init */
	char IPBUFF[20];

	GC.applic = new_app(argc,argv);
	
	GC.gui_win = new_window
				(
					GC.applic,
					rect(50,50,450,300),
					"SIDS CORRESPONDING NODE (tm)",
					STANDARD_WINDOW
				);
	GC.CNsLabel = new_label
				(
					GC.gui_win,
					rect(10,10,230,20),
					"CORRESPONDING NODE IP ADDR:",
					ALIGN_LEFT
				);
	set_control_background(GC.CNsLabel,LIGHT_GREY);
	
	GC.CNsText = new_field
				(
					GC.gui_win,
					rect(260,10,100,20),
					"2.0.0.1"
				);
	
	GC.TABPage = new_control
						(
							GC.gui_win,
							rect(10,40,430,105)
						);
	set_control_background(GC.TABPage,GREY);
	
	GC.NwkLabel= new_label
				(
					GC.gui_win,
					rect(44,52,60,15),
					"Network:",
					ALIGN_LEFT
				);
	set_control_background(GC.NwkLabel,LIGHT_BLUE);

	
	GC.NwkText = new_field(
							GC.gui_win,
							rect(110,50,100,20),
							""
						  );
	set_control_background(GC.NwkText,LIGHT_GREY);

	GC.InfLabel = new_label
				(
					GC.gui_win,
					rect(215,52,69,16),
					"Interface:",
					ALIGN_LEFT
				);
	set_control_background(GC.InfLabel,LIGHT_BLUE);

	GC.InfText = new_field
						(
							GC.gui_win,
							rect(290,50,100,20),
							""
						);
	set_control_background(GC.InfText,LIGHT_GREY);
	
	GC.IPLabel = new_label
				(
					GC.gui_win,
					rect(70,80,120,15),
					"IP packet sent to:",
					ALIGN_LEFT
				);
	set_control_background(GC.IPLabel,LIGHT_BLUE);

	GC.IPText = new_field
				(
					GC.gui_win,
					rect(200,78,160,20),
					NULL
				);
	set_control_background(GC.IPText,LIGHT_GREY);
	disable(GC.IPText);

	GC.sendButton = new_button
				(
					GC.gui_win,
					rect(55,120,320,20),
					"SEND IP PACKET NOW !",
					sendIpPacket
				);
	
	GC.MSGList = new_list_box 
				(
					GC.gui_win,
					 rect(10,150,430,140),
					 NULL,NULL
				);

    /* drawing lines */
	GC.topLineLeft = new_point(1,50);
	GC.topLineRight= new_point(20,50);

	GC.topLineG    = get_window_graphics(GC.gui_win);

	set_color(GC.topLineG,BLACK);
	draw_line(GC.topLineG,pt(1,20),pt(20,20));
	
	convertPortToIpFormat(atoi(argv[1]),atoi(argv[2]),IPBUFF);
	set_control_text (GC.CNsText,IPBUFF);


	show_window(GC.gui_win);
	main_loop(GC.applic);
	
	return 0;
}

