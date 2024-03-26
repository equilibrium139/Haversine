#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>

// <Startup=1, Read=2, Parse=3, Misc=4, ... >
// 

//enum {
//	X, Y, Z, ...
//};
//
// 		   X  Y  Z
//array[] = {0, 0, 0, ...}
//


class Profiler
{
	using u64 = std::uint64_t;
	using f64 = double;
	std::unordered_map<std::string, u64> categoryCycles;
	u64 startCount;
	u64 cpuFreq;
	u64 prevLogCount;
public:
	Profiler();
	void Log(const std::string& category);
	void PrintDiagnostics(std::ostream& out);
};