#include "common_include.h"



char* convertPortToIpFormat
	(
		int   Network,
		int   interface,
		char* buffer
	)
{
	if (Network >= 49)
		/* It means we purposly invalidate this interface so set its ip to 0.0.0.0 */
		sprintf(buffer,"%d.0.0.%d",0,0);	
	else
		sprintf(buffer,"%d.0.0.%d",Network,interface);	
	return buffer;
}


char* getTimeNow(char* timebuffer)
{
	time_t raw;
	struct tm*    ptm;

	time(&raw);
	ptm = gmtime (&raw);

	sprintf(timebuffer,"%2d:%02d:%02d",ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	return timebuffer;
}
