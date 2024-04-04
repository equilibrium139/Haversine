#pragma once

#include <cstdint>
#include <intrin.h>
#include <windows.h>

#ifndef READ_COUNTER
#define READ_COUNTER ReadCPUCounter
#endif

using u64 = std::uint64_t;

inline u64 ReadSystemCounter()
{
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return count.QuadPart;
}

inline u64 ReadSystemFreq()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

inline u64 ReadCPUCounter()
{
	return __rdtsc();
}

u64 ComputeCounterFreqFromSystemClock(u64 milliseconds = 1000);