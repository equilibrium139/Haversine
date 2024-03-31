#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "timing.h"

// <Startup=1, Read=2, Parse=3, Misc=4, ... >
// 

//enum {
//	X, Y, Z, ...
//};
//
// 		   X  Y  Z
//array[] = {0, 0, 0, ...}
//

using u64 = std::uint64_t;
using f64 = double;

class SimpleProfiler
{
	std::unordered_map<std::string, u64> categoryCycles;
	u64 startCount;
	u64 cpuFreq;
	u64 prevLogCount;
public:
	SimpleProfiler();
	void Log(const std::string& category);
	void PrintDiagnostics(std::ostream& out);
};

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)
#define VARNAME(Var) CAT(Var, __LINE__)

#define TimeFunction ScopeProfiler VARNAME(Profiler){__func__}
#define TimeBlock(name) ScopeProfiler VARNAME(Profiler){name}
					 
class ZoneProfiler
{
	u64 startCount;
	u64 cpuFreq;
	std::vector<std::pair<std::string, u64>> scopeCycles;
private:
	ZoneProfiler() {}
	friend struct ScopeProfiler;	
	static ZoneProfiler& Instance()
	{
		static ZoneProfiler* instance = new ZoneProfiler();
		return *instance;
	}
public:
	static void Start();
	static void EndAndPrintDiagnostics(std::ostream& out);
	ZoneProfiler(ZoneProfiler& other) = delete;
	void operator=(const ZoneProfiler&) = delete;
};

struct ScopeProfiler
{
	u64 startCount;
	std::string name;

	ScopeProfiler(const std::string& name)
		:startCount(ReadCPUCounter()), name(name) {}

	~ScopeProfiler()
	{
		u64 endCount = ReadCPUCounter();
		ZoneProfiler::Instance().scopeCycles.push_back({ name, endCount - startCount });
	}
};