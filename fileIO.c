#include "fileIO.h"


/*
  functions below are not important they simply read the config files

  These where initially used but have been surpased by fileIOUpdate
 */

int initConfigData()
{
	   /* initialising the CONFIG data */
	   int ix = 0;
	   for (ix = 0;ix < MAX_NEIGHBOURS_FILEIO ;ix++)
	   {
	   	  CONFIG_DATA[ix].validData = FALSE;
	   }
	return 0;
}

int initMobUnitPath()
{
 	int ix = 0;
	for (ix = 0;ix < MU_MAX_TRAVEL;ix++)
	{
		PATH_OF_MU[ix].validLocation = FALSE;
	}
	return 0;
}


int ReadThisRouterConfigFile(char* config_filename)
{
	int   END	   = 0;
	FILE* F_CONFIG = NULL;

	F_CONFIG = fopen(config_filename,"r");
	if (F_CONFIG == NULL)
	{
		printf("%s config file not found\n",config_filename);
		exit (ERR);
	}
	while (END != EOF)
	{
		END = fscanf 
				(
// RouterNumber|IP Addr of host|7000 or 6000 etc| 1/0 | 1/0
					F_CONFIG,"%d %s %d %d %d",
					&ThisRouterConfig.intRouterID,
					ThisRouterConfig.intHostRouterIsRunning,
					&ThisRouterConfig.intRouterSocketRange,
					&ThisRouterConfig.BOOLIsRouterHomeAgent,
					&ThisRouterConfig.BOOLIsRouterFrgnAgent
				);
		if(END == EOF)
		{
			break;
		}
	}
	fclose(F_CONFIG);
	return 0;
}

int ReadConfigFile(char* config_filename)
{
    int   END    = 0;	
	int   cnt    = 0;
	FILE* CONFIG = NULL;

	CONFIG = fopen(config_filename,"r");
	if (CONFIG == NULL)
	{
		printf("%s config file not found\n",config_filename);
		exit (ERR);
	}
	while(END != EOF)
	{
		if(
		    (
		        END=fscanf
				  (
// AddrOfRouter R1-R5| AddOfNwk N1-N5 | 7000-6000 etc| dummyip | dummyip
					CONFIG,"%d %d %d %s %s %s",
					&(CONFIG_DATA[cnt].intRouterAddress),
					&(CONFIG_DATA[cnt].intRouterNwk),
					&(CONFIG_DATA[cnt].intRouterSocketRange),
					 (CONFIG_DATA[cnt].chRouterIpDummy),
					 (CONFIG_DATA[cnt].chRouterNwk),
					 (CONFIG_DATA[cnt].chRouterRunningHost)
				  )
		    ) == EOF
		  )
		  {
					break;
		  }
		CONFIG_DATA[cnt].validData = TRUE;
	    cnt++;
	}
	fclose(CONFIG);
	return 0;
}
