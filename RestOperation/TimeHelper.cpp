#include "StdAfx.h"
#include "TimeHelper.h"
#include <time.h>
#include <sys/timeb.h>


/*get the millisecond count since midnight , January 1, 1970, UTC.
*/
INT64 TimeHelper::GetCurrentTimeStamp()
{
	struct _timeb currentTime;
	_ftime64_s(&currentTime);

	return (currentTime.time*1000 +currentTime.millitm);
}