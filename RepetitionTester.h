#pragma once

#include <cstdint>
#include <string>

class RepetitionTester
{
	enum State
	{
		Initializing, Testing, Complete
	};

	using u64 = std::uint64_t;
	using f64 = double;
	std::string name;
	f64 counterFreq;
	u64 testStartTSC;
	u64 slowestRunCycles;
	u64 fastestRunCycles;
	u64 totalCyclesElapsed;
	u64 runCount;
	u64 currentTestCycles;
	u64 minTestDurationCycles;
	u64 processedBytesPerRun;
	State state = State::Initializing;
	bool printNewFastestTimes;

public:
	RepetitionTester(const std::string& name, f64 minTestDurationSeconds, u64 processedBytesPerRun, bool printNewFastestTimes = false);
	void StartRun();
	void EndRun();
	void PrintDiagnostics();
	bool IsTesting() { return state != State::Complete; }
};