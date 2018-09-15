#include "GUIRouter.h"                      
#include "router.h"
static pthread_mutex_t GUIrouter_mtex;
int routerID = 0;
/*
	Call both these functions
 */
int initStructures()
{
	int ix = 0;	

	GR.RTRefreshCounter = 0;
	GR.ADListCounter    = 0;
	GR.MSGListCounter   = 0;
	
	RTableList[0] = malloc(sizeof(char)*50);
	RTableList[1] = malloc(sizeof(char)*50);
	RTableList[2] = malloc(sizeof(char)*50);
	RTableList[3] = malloc(sizeof(char)*50);
	RTableList[4] = malloc(sizeof(char)*50);
	//RTableList[5] = malloc(sizeof(char)*50);
	RTableList[5] = NULL;

	ADTableList[0] = calloc(50,sizeof(char));		
	ADTableList[1] = NULL;

	RIPList[0] = calloc(50,sizeof(char));
	RIPList[1] = NULL;

	for (ix = 0;ix < 99;ix++)
	{
		MSGList[ix] = calloc(200,sizeof(char));
	}
	MSGList[99] = NULL;

	for (ix = 0;ix < MAX_HOME_ADDRESS;ix++)
	{
		HAList[ix] = calloc(80,sizeof(char));
		strcpy(HAList[ix],"");
	}
	HAList[MAX_HOME_ADDRESS - 1]  = NULL;

	for (ix = 0;ix < MAX_CARE_OF_ADDRESS;ix++)
	{
		FAList[ix] = calloc(80,sizeof(char));
		strcpy(FAList[ix],"");
	}
	FAList[MAX_CARE_OF_ADDRESS-1] = NULL;

	return 0;
}



int createRouterGui(int argc,char** argv)
{
	int x_cord = 0;
	int y_cord = 0;
	char BUFF[30];
	if (argc < 3)
		exit (-1);
	
	routerID=atoi(argv[2]);	
	config_file = argv[1];
	GR.applic = new_app(argc,argv);

	x_cord = 900;y_cord = 400;
	GR.gui_win = new_window 
				(
					GR.applic,
					rect(50,150,x_cord,y_cord),
					"SIDSCO ROUTERS (tm)",
					STANDARD_WINDOW
				);
	
	GR.OnButton = new_button
				(
					GR.gui_win,
					rect(3,375,150,20),
					"START ROUTER",
					startRouter
				);
				
	GR.OffButton = new_button
				(
					GR.gui_win,
					rect(155,375,150,20),
					"STOP ROUTER",
					stopRouter
				);
	disable(GR.OffButton);

	GR.RTList = new_list_box
				(
					GR.gui_win,
					rect(8,55,350,100),
					RTableList,NULL
				);
	disable(GR.RTList);
	set_list_box_item(GR.RTList,-1);

	GR.ADList = new_list_box
				(
					GR.gui_win,
					rect(8,168,350,45),
					NULL,NULL
				);
	disable (GR.ADList);

	GR.RIPList = new_list_box
				(
					GR.gui_win,
					rect(8,215,350,45),
					NULL,NULL
				);
	disable(GR.RIPList);
	
	GR.MSGList = new_list_box
				(
					GR.gui_win,
					rect(8,270,880,90),
					NULL,NULL
				);
	/*
	GR.FAcheckbox = new_check_box
				(
					GR.gui_win,
					rect(350,15,200,40),
					"FOREIGN AGENT",
					NULL
				);
	disable(GR.FAcheckbox);
	GR.HAcheckbox = new_check_box
				(
					GR.gui_win,
					rect(500,15,150,40),
					"HOME AGENT",
					NULL
				);
	disable(GR.HAcheckbox);
	*/
	sprintf(BUFF,"[ ROUTER %d ]",routerID);
    GR.RouterID  = new_field
				(
					GR.gui_win,
					rect(350,15,150,20),
					BUFF
				);
	disable(GR.RouterID);
	GR.RTRStatus = new_label
				(
					GR.gui_win,
					rect(455,378,200,40),
					"ROUTER STATUS",
					ALIGN_LEFT
				);

	GR.RTRStatusOn = new_radio_button
				(
					GR.gui_win,
					rect(590,378,100,40),
					"ON",
					NULL
				);

	disable(GR.RTRStatusOn);	
	GR.RTRStatusOff = new_radio_button
				(
					GR.gui_win,
					rect(640,378,100,40),
					"OFF",
					NULL
				);
	disable(GR.RTRStatusOff);
	
	GR.RTRName = new_label
				(
					GR.gui_win,
					rect(30,15,300,100),
					"SIDSCO ROUTER 2160 SeriesII",
					ALIGN_LEFT
				);
				
	/*----------------------------------------------------------*/

	GR.TabPage[1] = new_control(
									GR.gui_win,
									rect(365,90,522,170)
							   );
	set_control_background(GR.TabPage[1],GREY);

	GR.FALabel = add_label  (
								GR.TabPage[1],
								rect(5,5,200,15),
								"Foreign Agent Special Table",
								ALIGN_LEFT
							 );
	//set_control_background(GR.FALabel,LIGHT_GREY);

	GR.FATable = add_list_box
							(
								GR.TabPage[1],
								rect(4,22,513,130),
								NULL,NULL
							);
	disable(GR.FATable);
	GR.FASelectB = new_button
				(
					GR.gui_win,
					rect(516,56,180,33),
					"FOREIGN AGENT TABLE",
					selectTablePane
				);
	set_control_value(GR.FASelectB,1);
	set_control_background(GR.FASelectB,GREY);

	// control seperator 

	GR.HASelectB = new_button
				(
					GR.gui_win,
					rect(365,56,150,33),
					"HOME AGENT TABLE",
					selectTablePane
				);
	set_control_value(GR.HASelectB,0);
	set_control_background(GR.HASelectB,GREY);
	GR.TabPage[0] = new_control(
									GR.gui_win,
									rect(365,90,522,170)
							   );
	set_control_background(GR.TabPage[0],GREY);
	GR.HALabel = add_label  (
								GR.TabPage[0],
								rect(5,5,180,15),
								"Home Agent Special Table",
								ALIGN_LEFT
							 );
	//set_control_background(GR.HALabel,LIGHT_GREY);
	set_control_background(GR.TabPage[0],GREY);


	GR.HATable = add_list_box
							(
								GR.TabPage[0],
								rect(4,22,513,130),
								NULL,NULL
							);
	disable(GR.HATable);
	/*----------------------------------------------------------*/
	set_control_background(GR.TabPage[0],GREY);


	show_window(GR.gui_win);
	main_loop(GR.applic);
	return 0;
							 
}

/*----------------------------------------------------------*/
void selectTablePane(Control* ctrl)
{
	int pane_sel = get_control_value(ctrl);
	if (pane_sel)
	{
		show_control(GR.TabPage[pane_sel]);
		hide_control(GR.TabPage[0]);
	}
	else
	{
		show_control(GR.TabPage[pane_sel]);
		hide_control(GR.TabPage[1]);
	}
	redraw_control(GR.TabPage[pane_sel]);
}
/*----------------------------------------------------------*/

int addToRouterList(RoutingTableOBJ* RT)
{
	int  ix = 0i,MAX = 0;
	int  LBindex = 0;
	char digit[20];
	MAX = RT->MAX_ARRAY_SIZE;
	
	pthread_mutex_lock(&GUIrouter_mtex);

	for(ix =0;ix < 6;ix++)
	{
		if(((RoutingTable)RT->RTArray[ix]).HasFieldBeenTaken)
		{
			sprintf(digit,"%d",RT->RTArray[ix].intDistance);
			sprintf (
						RTableList[LBindex],"DNwk:%s | NHop:%s | Dist:%s",
						RT->RTArray[ix].chDestination,
						RT->RTArray[ix].chRoute,
					   (RT->RTArray[ix].intDistance >= 12)?("INF"):(digit)
					);	
			change_list_box(GR.RTList,RTableList);
			LBindex++;
		}
	}
	pthread_mutex_unlock(&GUIrouter_mtex);
	return 0;
}
/*----------------------------------------------------------*/

int addToAdvertList(char* line)
{
	strcpy(ADTableList[GR.ADListCounter],line);
	change_list_box(GR.ADList,ADTableList);
	redraw_control(GR.ADList);

	return 0;
}

/*----------------------------------------------------------*/

int  addToMSGList (char* msgs,int RTEntry, PACKET_size_t PKT)
{
	char BUFF[200];
	char timebuff[20];

	getTimeNow(timebuff);

	if (RTEntry != ERR)
	{
		int NetID    = RIP_checkNetworkID(PKT);
		int NextHop  = RIP_checkSrcAddrROUTER(PKT);
		int Dist     = RIP_checkDistanceToNwk(PKT);

		
		switch (RTEntry)
		{
			case RT_NOT_EXIST:
				 sprintf(BUFF,"[%s]:RT Update | NOT EXIST | ADD:N%d | NHop:%d | Dist:%d | PKT SRC = %d",
				 			   timebuff,NetID,NextHop,Dist+1,NextHop);
				 break;

			case RT_HOP_SAME_CHANGE:
				 sprintf(BUFF,"[%s]:RT Update | SAME NHOP | UPDATE:N%d | NHop:%d | Dist:%d | PKT SRC = %d",
				 			   timebuff,NetID,NextHop,Dist+1,NextHop);
				 break;

			case RT_HOP_SHORTER:
				 sprintf(BUFF,"[%s]:RT Update | NHOP SHRTR | UPDATE:N%d | NHop:%d | Dist:%d | PKT SRC = %d",
				 			   timebuff,NetID,NextHop,Dist+1,NextHop);
				 break;
		}
		if (GR.MSGListCounter > 98)
		{
			/* we should clear the list here */
			GR.MSGListCounter = 0;
		}

		strcpy(MSGList[GR.MSGListCounter],BUFF);
		change_list_box(GR.MSGList,MSGList);
		
		GR.MSGListCounter++;
		return 0;
	}
	/* here is where we handle other mesages */
	if (GR.MSGListCounter > 98)
		GR.MSGListCounter = 0;
	
	sprintf(BUFF,"[%s]: %s",timebuff,msgs);
	strcpy(MSGList[GR.MSGListCounter],BUFF);
	change_list_box(GR.MSGList,MSGList);
	
	GR.MSGListCounter++;
	return 0;	
}

/*----------------------------------------------------------*/

int addToHASpecTable (HARobj* HAO)
{
	int  ix = 0;
	int  inner = 0;

	char HMA[20];
	char COA[20];
	char FNK[20];
	char HNK[20];
	
	char timebuff[20];

	getTimeNow (timebuff);
	for(ix = 0;ix < HAO->MAX_ARRAY_SIZE; ix++)
	{
		HomeAgentsRegistered HR = HAO->HARArray[ix];
		if (
			HR.IsFieldBeingUsed 
		   )
		{
		    convertPortToIpFormat (	HR.HomeNetworkAddress, 0,HNK);
			convertPortToIpFormat ( HR.HomeNetworkAddress,HR.HomeAddress,HMA);
			convertPortToIpFormat ( HR.ForeignAgentAddress,0,FNK);
			convertPortToIpFormat ( HR.ForeignAgentAddress,HR.CareOfAddress,COA);
			sprintf
			(
				HAList [ inner ],"[%s]:HME_Nwk: %s | HMA: %s | FRG_Nwk: %s | CoA: %s",
				timebuff,HNK,HMA,FNK,COA
			);
			change_list_box (GR.HATable,HAList);
			inner++;	
		}
		else
		{
			strcpy (HAList [ inner ],"");
			
		}
	}
	return 0;
}

/*----------------------------------------------------------*/

int addToRIPList(int thisRouter,int frgnNtwk)
{
	char timebuff[20];

	getTimeNow(timebuff);
	sprintf(RIPList[0],"[%s]:RIP Table From RTR%d To N%d Sent \n",
						timebuff,thisRouter,frgnNtwk);	
	
	change_list_box (GR.RIPList,RIPList);
	return 0;
}

/*----------------------------------------------------------*/

int  addToFASpecTable (FARObj* FA0, int this_nwk)
{
	int ix = 0;
	int inner = 0;

	char HMA[20];
	char COA[20];
	char HNK[20];
	
	char timebuff[20];
	
	getTimeNow(timebuff);
	for (ix = 0;ix < MAX_CARE_OF_ADDRESS -1;ix++)
	{
		ForeignAgentsRegistered FR = FA0->FAHArray[ix];
		if (FR.IsFieldBeingUsed )
		{
			// we still need to add time
			// need to add the time as the first field
			if (FR.IsStillRegistered)
			{
				convertPortToIpFormat (this_nwk,FR.CareOfAddress,COA);
				convertPortToIpFormat (FR.HomeAgentAddress,FR.HomeAddress,HMA);
				convertPortToIpFormat (FR.HomeAgentAddress,0,HNK);

				sprintf
				(
					FAList [ inner ],"[%s]:COA: %s | HMA: %s | LIFE: %d secs | HA: %s",
					timebuff,COA,HMA,FR.MobileUnitLifeTime,HNK
				);
			}
			else
			{
				strcpy (FAList [ inner ],"");
			}
			change_list_box (GR.FATable,FAList);
			inner++;
		}
		else
		{
			strcpy (FAList [ inner ],"");
			change_list_box (GR.FATable,FAList);
		}
	}
	return 0;
}

/*----------------------------------------------------------*/
void startRouter(Control* c)
{
	/* must check return value just a quick hack here */
	pthread_create(&GR.ROUTER_THREAD,NULL,startRouterWrapper,NULL);
	enable(GR.OffButton);
	check   (GR.RTRStatusOn);
	uncheck (GR.RTRStatusOff);
	disable(c);
}

void stopRouter(Control* c)
{
	pthread_cancel(GR.ROUTER_THREAD);
	pthread_cancel(ThisRouter->icmpDispatchThreadID);
	pthread_cancel(ThisRouter->tableDispatchThreadID);
	
	check   (GR.RTRStatusOff);
	uncheck (GR.RTRStatusOn);
	
	close (CONNECTChan->ServerSocket);
	enable(GR.OnButton);
	disable(c);
}

void* startRouterWrapper(void* param)
{
	ROUTER_init(config_file);
	return NULL;
}

