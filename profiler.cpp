#include "profiler.h"
#include "timing.h"
#include <cassert>
#include <iomanip>

using f64 = double;

SimpleProfiler::SimpleProfiler()
	:startCount(ReadCPUCounter()), cpuFreq(ComputeCPUFreqFromSystemClock(10)), prevLogCount(startCount)
{
}

void SimpleProfiler::Log(const std::string& category)
{
	u64 currentCount = ReadCPUCounter();
	u64 cyclesElapsed = currentCount - prevLogCount;
	prevLogCount = currentCount;
	categoryCycles[category] += cyclesElapsed;
}

void SimpleProfiler::PrintDiagnostics(std::ostream& out)
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

void ZoneProfiler::Start()
{
	ZoneProfiler::Instance().startCount = ReadCPUCounter();
	ZoneProfiler::Instance().cpuFreq = ComputeCPUFreqFromSystemClock(10);
}

void ZoneProfiler::EndAndPrintDiagnostics(std::ostream& out)
{
	u64 currentCount = ReadCPUCounter();
	ZoneProfiler& instance = ZoneProfiler::Instance();
	u64 totalCycles = currentCount - instance.startCount;
	u64 miscCycles = totalCycles;
	f64 msEslaped = totalCycles / (instance.cpuFreq / 1000);
	out << "Total time: " << msEslaped << "ms (CPU freq " << instance.cpuFreq << ")\n";
	out << std::fixed;
	out << std::setprecision(2);
	for (const auto& scope : instance.scopeCycles)
	{
		out << scope.first << ": " << scope.second << "(" << (f64)scope.second / (f64)totalCycles * 100.0 << "%)\n";
		miscCycles -= scope.second;
	}
	out << "Misc: " << miscCycles << "(" << (f64)miscCycles / (f64)totalCycles * 100.0 << "%)\n";
}