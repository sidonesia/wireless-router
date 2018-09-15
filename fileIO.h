#ifndef FILEIO_H
#define FILEIO_H

#include "common_include.h"

#define  MAX_NEIGHBOURS_FILEIO 5
#define  MU_MAX_TRAVEL         15
/*
 * represents the fields in the configuration file
 * representing this routers neighbours
 */

 /*
  * We need another configuration file for 
  * this router on top of the config files for
  * the other routers
  */

  /*
   * we also need another configuration file for
   * that is identical on all routers this is for
   * things which need to be the same everywhere
   */

/* holds information on the current location */
typedef struct
{
	BOOLEAN validLocation;
	int CurrentNetwork;
	int ResidenceTime;
} 
point_location;

/* contains info about th mobile unit */
typedef struct
{
	int  NwkMultFrgn; /* foreign agent the MU is on */
	int  NwkMultHme;  /* home agent for the MU*/
	int  MUHmeNwk;    /* home address of the mu*/
	int  RtrRange;    /* the range of addresses the routers can have */
	char IPAddrHost[50];
}
MUConfigData;

/* contains info on the router */
typedef struct
{
    int    validData; /* simple flag to see if stucture is filled */

	int    intRouterAddress; /* router id*/
	int    intRouterNwk;     /* network the router is on*/
	int    intRouterSocketRange; /* the range of ports the router can have 7000, 8000, 9000 are ranges*/

	char  chRouterIpDummy[50]; 
	char  chRouterNwk[50]; 
	char  chRouterRunningHost[50]; /* shows which host the router is running on */
}FileData;

/* represents info that stored in the config file for a router */
typedef struct
{
	int		intRouterID;
	char 	intHostRouterIsRunning[50];
	int     intRouterSocketRange;
	BOOLEAN BOOLIsRouterHomeAgent;
	BOOLEAN BOOLIsRouterFrgnAgent;
}RouterData;

FileData 	 CONFIG_DATA[MAX_NEIGHBOURS_FILEIO];
RouterData   ThisRouterConfig;
MUConfigData ThisMUConfig; 
point_location PATH_OF_MU[MU_MAX_TRAVEL];       

int initConfigData(); /* will initialise all the structures before reading */
int initMobUnitPath(); /* same as above for the MU's files */
int ReadConfigFile(char* config_filename); /* will read the Mu's config file */
int ReadThisRouterConfigFile(char* config_filename); /* will read the routers config file */

#endif
