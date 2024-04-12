#include "Timing.h"

u64 ComputeCounterFreqFromSystemClock(u64 milliseconds)
{
	u64 systemFreq = ReadSystemFreq();
	u64 systemCycles = (systemFreq * milliseconds) / 1000;
	u64 duration = 0;
	u64 counterStart = READ_COUNTER();
	u64 systemStart = ReadSystemCounter();
	while (duration < systemCycles)
	{
		duration = ReadSystemCounter() - systemStart;
	}
	u64 counterEnd = READ_COUNTER();
	u64 counterCycles = counterEnd - counterStart;
	u64 counterFreq = (counterCycles * 1000) / milliseconds;
	return counterFreq;
}