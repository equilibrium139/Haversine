#include "RepetitionTester.h"
#include "Timing.h"
#include <iostream>
#include <format>

RepetitionTester::RepetitionTester(const std::string& name, f64 minTestDurationSeconds, u64 processedBytesPerRun, bool printNewFastestTimes)
	:name(name), counterFreq(ComputeCounterFreqFromSystemClock(100)), slowestRunCycles(0), fastestRunCycles(UINT64_MAX), 
	 totalCyclesElapsed(0), runCount(0), currentTestCycles(0), minTestDurationCycles(minTestDurationSeconds * counterFreq), 
	 processedBytesPerRun(processedBytesPerRun), printNewFastestTimes(printNewFastestTimes)
{
	std::cout << "Repetition testing started for \"" << name << "\"\n";
}

void RepetitionTester::StartRun()
{
	if (state == State::Initializing)
	{
		testStartTSC = READ_COUNTER();
		state = State::Testing;
	}

	currentTestCycles -= READ_COUNTER();
}

void RepetitionTester::EndRun()
{
	currentTestCycles += READ_COUNTER();
	totalCyclesElapsed += currentTestCycles;
	runCount++;

	if (currentTestCycles > slowestRunCycles)
	{
		slowestRunCycles = currentTestCycles;
	}
	if (currentTestCycles < fastestRunCycles)
	{
		fastestRunCycles = currentTestCycles;

		if (printNewFastestTimes)
		{
			std::cout << "New fastest time: " << (f64)fastestRunCycles / (f64)counterFreq * 1000.0 << "ms\n";
		}

		// Restart test timer when a new fastest time is encountered
		testStartTSC = READ_COUNTER();
	}
	u64 testDurationCycles = READ_COUNTER() - testStartTSC;
	if (testDurationCycles > minTestDurationCycles)
	{
		state = State::Complete;
	}
	currentTestCycles = 0;
}

void RepetitionTester::PrintDiagnostics()
{
	f64 gigabytesPerRun = processedBytesPerRun / (1024.0 * 1024.0 * 1024.0);

	f64 slowestRunSeconds = slowestRunCycles / counterFreq;
	f64 slowestRunMS = slowestRunSeconds * 1000.0;
	f64 slowestRunThroughput = gigabytesPerRun / slowestRunSeconds;

	f64 fastestRunSeconds = fastestRunCycles / counterFreq;
	f64 fastestRunMS = fastestRunSeconds * 1000.0;
	f64 fastestRunThroughput = gigabytesPerRun / fastestRunSeconds;

	f64 averageCyclesPerRun = (f64)totalCyclesElapsed / (f64)runCount;
	f64 averageSecondsPerRun = averageCyclesPerRun / counterFreq;
	f64 averageMSPerRun = averageSecondsPerRun * 1000.0;
	f64 averageRunThroughput = gigabytesPerRun / averageSecondsPerRun;

	std::cout << 
		std::format("Diagnostics for {}:\nSlowest run: {} cycles {}ms {}gb/s\nFastest run: {} cycles {}ms {}gb/s\nAverage: {} cycles {}ms {}gb/s\n\n", 
		name, slowestRunCycles, slowestRunMS, slowestRunThroughput, fastestRunCycles, fastestRunMS, fastestRunThroughput,
		averageCyclesPerRun, averageMSPerRun, averageRunThroughput);
}
