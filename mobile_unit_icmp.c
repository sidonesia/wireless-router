#include "mobile_unit_icmp.h"

/*------------------------------------------------------------
 * will set the details of the home agent to allow
 * communication between MU and HA
 ------------------------------------------------------------*/
int MUNIT_ICMP_setHomeAgent
	(
		MobileUnit_t* MU,
		PACKET_size_t ICMP_PKT
	)
{
	int HAaddress = ICMP_checkSourceAddressOfPacket(ICMP_PKT);
#ifdef DEBUG_ME	
	printf("[MOBILE UNIT]: HOME ADDRESS PORT DEFINED AS: %d\n",HAaddress+MU->RouterRanges);
#endif
	MU->HAgentDetails.MobUnitHomeAgentNwk = HAaddress;
	MU->HomeAddressRouterPort = HAaddress + MU->RouterRanges;
	return 0;	
}

/*-----------------------------------------------------------
 * will set the details of the foreign agent to allow
 * communication between MU and FA
 ------------------------------------------------------------*/
int MUNIT_ICMP_setFrgnAgent
	(
		MobileUnit_t* MU,
		PACKET_size_t ICMP_PKT
	)
{
	int  FAaddress = ICMP_checkSourceAddressOfPacket(ICMP_PKT);
	int  LifeTime  = ICMP_checkLifeTime(ICMP_PKT);
	char timeBUFF[40];

	MU->LifeTimeOfReg = LifeTime;
	MU->FrgnNetwork   = FAaddress;
	MU->FrgnAddressRouterPort = FAaddress + MU->RouterRanges;
#ifdef DEBUG_ME
	printf("[MOBILE UNIT]: DISPATCHING TO ROUTER ON PORT: %d \n",MU->FrgnAddressRouterPort);
#endif
	getTimeNow(timeBUFF);
	printf("[MOBILE UNIT]@[%s]: LIFETIME OF THE MOBILE UNIT ON THE FA IS: %d\n",timeBUFF,LifeTime);
	return 0;
}

PACKET_size_t MUNIT_ICMP_buildICMPPkt
	(
		MobileUnit_t* MU
	)
{
	return 0;
}
