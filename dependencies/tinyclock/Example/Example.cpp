#include <stdio.h>
#include <TinyClock.h>
int main()
{
	tinyClock_t exampleClock = tinyClock_t();

	for (double Time = 0; Time < 10; Time = exampleClock.GetTotalTime())
	{		
		exampleClock.UpdateClockAdaptive();
		printf("%f \r", exampleClock.GetTotalTime());
	}

	return 0;
}
