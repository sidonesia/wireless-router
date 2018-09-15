#include "common_include.h"
#include "fileIO.h"

/*-------------------------------------------------------*
 * This function will read the configuration file for    *
 * The mobile unit                                       *
 *-------------------------------------------------------*/
int ScanMUConfigFileFirst(char* filename)
{
	FILE* cfgFile = fopen(filename,"r");
	char  CFG_BUFF[512];
	int   READ_PATH = 0;

	int   path_cnt = 0;

	if (cfgFile == NULL)
	{
		printf("MU: configuration file : %s not found, exiting...\n",filename);
		exit (-1);
	}

	while (fgets(CFG_BUFF,512,cfgFile) != NULL)
	{
		if (CFG_BUFF[0] != '#' && CFG_BUFF[0] != '\n') 
		{
			if (strstr(CFG_BUFF,"MU_PATH") != NULL)	
				READ_PATH = 1;
			
			if (READ_PATH && (strstr(CFG_BUFF,"MU_PATH") == NULL))
			{
				// use a counter, read the path 
				sscanf
				   (
				   	 CFG_BUFF, "%d %d",
					 &PATH_OF_MU[path_cnt].CurrentNetwork,
					 &PATH_OF_MU[path_cnt].ResidenceTime
				   );


				PATH_OF_MU[path_cnt].validLocation = TRUE;
				path_cnt++;
			}
			else
			if (!(READ_PATH) && (strstr(CFG_BUFF,"THIS_MU") == NULL))
			{
				// read in the details of the router 
				sscanf
				    (
						CFG_BUFF,"%d %s %d %d %d",
						&ThisMUConfig.NwkMultFrgn,
						ThisMUConfig.IPAddrHost,
						&ThisMUConfig.MUHmeNwk,
						&ThisMUConfig.NwkMultHme,
						&ThisMUConfig.RtrRange
						
					);
			}

		}
	}
	return 0;

}


/*---------------------------------------------------------*
 * This function will read the configuration file for      *
 * The routers.                                            *
 *---------------------------------------------------------*/
int ScanConfigFileFirst(char* filename)
{
	FILE* cfgFile = fopen(filename,"r");
	char  CFG_BUFF[512];
	int   READ_NBR= 0;

	int ngbr_cnt = 0;
	
	if (cfgFile == NULL)
	{
		printf("RTR: configuration file : %s not found, exiting...\n",filename);
		exit (-1);
	}

	while (fgets(CFG_BUFF,512,cfgFile) != NULL)
	{
		if (CFG_BUFF[0] != '#' && CFG_BUFF[0] != '\n') 
		{
			if (strstr(CFG_BUFF,"NGBR_ROUTER") != NULL)	
				READ_NBR = 1;
			
			if (READ_NBR && (strstr(CFG_BUFF,"NGBR_ROUTER") == NULL))
			{
				// call the format to 
				// read this part of the config file
				// use sscanf !!!
				printf(" In the READ_NGBR = 1: %s",CFG_BUFF);
				sscanf
						(
								CFG_BUFF,"%d %d %d %s %s %s",
								&(CONFIG_DATA[ngbr_cnt].intRouterAddress),
								&(CONFIG_DATA[ngbr_cnt].intRouterNwk),
								&(CONFIG_DATA[ngbr_cnt].intRouterSocketRange),
								 (CONFIG_DATA[ngbr_cnt].chRouterIpDummy),
								 (CONFIG_DATA[ngbr_cnt].chRouterNwk),
								 (CONFIG_DATA[ngbr_cnt].chRouterRunningHost)
						);
				CONFIG_DATA[ngbr_cnt].validData = TRUE;
				ngbr_cnt++;
			}
			else
			if (!(READ_NBR) && (strstr(CFG_BUFF,"THIS_ROUTER") == NULL))
			{
				// call the format to 
				// read this part of the config file
				// use sscanf !!!
				printf(" In the READ_NGBR = 0: %s",CFG_BUFF);
				sscanf
					(
					   CFG_BUFF,"%d %s %d %d %d",
					   &ThisRouterConfig.intRouterID,
					   ThisRouterConfig.intHostRouterIsRunning,
					   &ThisRouterConfig.intRouterSocketRange,
					   &ThisRouterConfig.BOOLIsRouterHomeAgent,
					   &ThisRouterConfig.BOOLIsRouterFrgnAgent
					);
			}
		}
	}
	return 0;
}


