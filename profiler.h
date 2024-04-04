#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
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

#define TimeFunction ScopeProfiler VARNAME(Block){__func__}
#define TimeBlock(name) ScopeProfiler VARNAME(Block){name}

struct ZoneData
{
	u64 cycles = 0;
	u64 childrenCycles = 0;
	bool hasActiveInstance = false;
};
					 
class ZoneProfiler
{
	u64 startCount;
	u64 cpuFreq;
	std::unordered_map<std::string, ZoneData> zones;
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

inline ScopeProfiler* currentScope = nullptr;

struct ScopeProfiler
{
	u64 startCount;
	u64 childrenCycles;
	std::string name;
	ScopeProfiler* parent = nullptr;
	ZoneData* zone;
	bool isRecursiveCall = false;

	ScopeProfiler(const std::string& name)
		:startCount(ReadCPUCounter()), childrenCycles(0), name(name), parent(currentScope), zone(&ZoneProfiler::Instance().zones[name])
	{
		isRecursiveCall = zone->hasActiveInstance;
		zone->hasActiveInstance = true;
		currentScope = this;
	}

	~ScopeProfiler()
	{
		u64 cycles = ReadCPUCounter() - startCount;
		if (parent && parent->zone != zone) parent->childrenCycles += cycles;
		if (!isRecursiveCall) {
			zone->cycles += cycles;
			zone->childrenCycles += childrenCycles;
			zone->hasActiveInstance = false;
		}
		else
		{
			zone->childrenCycles -= cycles - childrenCycles;
		}
		currentScope = parent;
	}
};