#include "timing.h"

u64 ComputeCPUFreqFromSystemClock(u64 milliseconds)
{
	u64 systemFreq = ReadSystemFreq();
	u64 systemCycles = (systemFreq * milliseconds) / 1000;
	u64 duration = 0;
	u64 cpuStart = ReadCPUCounter();
	u64 systemStart = ReadSystemCounter();
	while (duration < systemCycles)
	{
		duration = ReadSystemCounter() - systemStart;
	}
	u64 cpuEnd = ReadCPUCounter();
	u64 cpuCycles = cpuEnd - cpuStart;
	u64 cpuFreq = (cpuCycles * 1000) / milliseconds;
	return cpuFreq;
}