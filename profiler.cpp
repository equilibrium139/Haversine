#include "profiler.h"
#include "timing.h"
#include <cassert>
#include <iomanip>
#include <format>

using f64 = double;

SimpleProfiler::SimpleProfiler()
	:startCount(READ_COUNTER()), counterFreq(ComputeCounterFreqFromSystemClock(10)), prevLogCount(startCount)
{
}

void SimpleProfiler::Log(const std::string& category)
{
	u64 currentCount = READ_COUNTER();
	u64 cyclesElapsed = currentCount - prevLogCount;
	prevLogCount = currentCount;
	categoryCycles[category] += cyclesElapsed;
}

void SimpleProfiler::PrintDiagnostics(std::ostream& out)
{
	u64 currentCount = READ_COUNTER();
	
	u64 totalCycles = currentCount - startCount;
	u64 miscCycles = totalCycles;
	f64 msEslaped = totalCycles / (counterFreq / 1000);
	out << "Total time: " << msEslaped << "ms (counter freq " << counterFreq << ")\n";
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
	ZoneProfiler::Instance().startCount = READ_COUNTER();
	ZoneProfiler::Instance().counterFreq = ComputeCounterFreqFromSystemClock(100);
}

void ZoneProfiler::EndAndPrintDiagnostics(std::ostream& out)
{
	u64 currentCount = READ_COUNTER();
	ZoneProfiler& instance = ZoneProfiler::Instance();
	u64 totalCycles = currentCount - instance.startCount;
	u64 miscCycles = totalCycles;
	f64 msEslaped = totalCycles / (instance.counterFreq / 1000);
	out << "Total time: " << msEslaped << "ms (counter freq " << instance.counterFreq << ")\n";
	out << "Total cycles: " << totalCycles << '\n';
	out << std::fixed;
	out << std::setprecision(2);
	for (const auto& scope : instance.zones)
	{
		u64 cyclesExcludingChildren = scope.second.cycles - scope.second.childrenCycles;
		out << scope.first << "[" << scope.second.callCount << "]: " << scope.second.cycles << "(" << (f64)cyclesExcludingChildren / (f64)totalCycles * 100.0 << "%";
		if (scope.second.childrenCycles > 0)
		{
			out << ", " << (f64)scope.second.cycles / (f64)totalCycles * 100.0 << "% w/ children";
		}
		out << ") ";
		if (scope.second.processedByteCount > 0)
		{
			f64 seconds = (f64)scope.second.cycles / (f64)instance.counterFreq;
			f64 megabytes = (f64)scope.second.processedByteCount / (1024.0 * 1024.0);
			f64 gigabytes = megabytes / 1024.0;
			f64 gigabytesPerSecond = gigabytes / seconds;
			out << std::format("processed {:.3f}MB at {:.2f}GB/s", megabytes, gigabytesPerSecond);
		}
		out << "\n";

		miscCycles -= cyclesExcludingChildren;
	}
	out << "Misc: " << miscCycles << "(" << (f64)miscCycles / (f64)totalCycles * 100.0 << "%)\n";
}