#include "profiler.h"
#include "timing.h"
#include <iomanip>

Profiler::Profiler()
	:startCount(ReadCPUCounter()), cpuFreq(ComputeCPUFreqFromSystemClock(10)), prevLogCount(startCount)
{
}

void Profiler::Log(const std::string& category)
{
	u64 currentCount = ReadCPUCounter();
	u64 cyclesElapsed = currentCount - prevLogCount;
	prevLogCount = currentCount;
	categoryCycles[category] += cyclesElapsed;
}

void Profiler::PrintDiagnostics(std::ostream& out)
{
	u64 currentCount = ReadCPUCounter();
	
	u64 totalCycles = currentCount - startCount;
	u64 miscCycles = totalCycles;
	f64 msEslaped = totalCycles / (cpuFreq / 1000);
	out << "Total time: " << msEslaped << "ms (CPU freq " << cpuFreq << ")\n";
	out << std::fixed;
	out << std::setprecision(2);
	for (const auto& category : categoryCycles)
	{
		out << category.first << ": " << category.second << "(" << (f64)category.second / (f64)totalCycles * 100.0 << "%)\n";
		miscCycles -= category.second;
	}
	out << "Misc: " << miscCycles << "(" << (f64)miscCycles / (f64)totalCycles * 100.0 << "%)\n";
}
