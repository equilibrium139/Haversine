#define _CRT_SECURE_NO_DEPRECATE // for fread
#define NOMINMAX
#include <stdio.h>
#include <cassert>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#define PROFILER 1
#include "Profiler.h"
#include "Timing.h"
#include "RepetitionTester.h"


using f64 = double;
using u64 = std::uint64_t;

struct Pair
{
	f64 x0, y0, x1, y1;
};

constexpr f64 radius = 6372.8;

static std::vector<Pair> ParseJson(const char* filename);
static f64 Square(f64 A);
static f64 RadiansFromDegrees(f64 Degrees);
// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(const Pair& pair, f64 EarthRadius);

struct HaversineResult
{
	std::vector<f64> distances;
	f64 average;
};

static HaversineResult SumPairs(const std::vector<Pair>& pairs);

static void TestReadFunctions(const char* filename);

int main(int argc, char** argv)
{
	ZoneProfiler::Start();
	SimpleProfiler profiler;

	if (argc < 2)
	{
		std::cout << "Usage: haversine json_file [answer_file]\n";
		return 0;
	}

	char* jsonFilename = argv[1];
	char* answerFilename = nullptr;
	if (argc == 3)
	{
		answerFilename = argv[2];
	}

	profiler.Log("Startup");

	std::vector<Pair> pairs = ParseJson(jsonFilename);

	profiler.Log("Read and parse JSON");

	HaversineResult result = SumPairs(pairs);
	std::cout << "Average: " << result.average << '\n';

	profiler.Log("Sum");

	if (answerFilename != nullptr)
	{
		TimeBlock("Process answer file");
		std::ifstream answerFile{ answerFilename, std::ios::binary };
		auto p = std::filesystem::current_path().append(answerFilename);
		auto answerFileSizeBytes = std::filesystem::file_size(p);
		std::vector<f64> answerFileContents(answerFileSizeBytes / 8);
		answerFile.read((char*)&answerFileContents[0], answerFileSizeBytes);
		std::cout << "Answer file average: " << answerFileContents.back() << '\n';
	}

	profiler.Log("Process answer file");

	TestReadFunctions(jsonFilename);

	//profiler.PrintDiagnostics(std::cout);
	ZoneProfiler::EndAndPrintDiagnostics(std::cout);
}

// Makes a lot of assumptions about how JSON file is structured
std::vector<Pair> ParseJson(const char* filename)
{
	TimeFunction;

	std::vector<Pair> pairs;
	std::vector<std::string> lines;

	auto filepath = std::filesystem::current_path().append(filename);
	auto jsonFileBytes = std::filesystem::file_size(filepath);

	{
		TimeBlockBandwidth("Load Json file", jsonFileBytes);
		std::ifstream jsonFile{ filename };
		std::string line;
		while (std::getline(jsonFile, line))
		{
			lines.push_back(line);
		}
	}

	{
		TimeBlock("Parse Json file");
		for (u64 i = 0; i < lines.size(); i++)
		{
			const std::string& line = lines[i];
			if (line.size() > 2)
			{
				pairs.emplace_back();
				Pair& pair = pairs.back();
				std::size_t offset = line.find(':') + 1;
				std::from_chars(&line[offset], &line.back(), pair.x0);
				offset = line.find(':', offset) + 1;
				std::from_chars(&line[offset], &line.back(), pair.y0);
				offset = line.find(':', offset) + 1;
				std::from_chars(&line[offset], &line.back(), pair.x1);
				offset = line.find(':', offset) + 1;
				std::from_chars(&line[offset], &line.back(), pair.y1);
			}
		}
		return pairs;
	}
}

f64 Square(f64 A)
{
	f64 Result = (A * A);
	return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
	f64 Result = 0.01745329251994329577 * Degrees;
	return Result;
}

f64 ReferenceHaversine(const Pair& pair, f64 EarthRadius)
{
	/* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
	   Instead, it attempts to follow, as closely as possible, the formula used in the real-world
	   question on which these homework exercises are loosely based.
	*/
	f64 lat1 = pair.y0;
	f64 lat2 = pair.y1;
	f64 lon1 = pair.x0;
	f64 lon2 = pair.x1;

	f64 dLat = RadiansFromDegrees(lat2 - lat1);
	f64 dLon = RadiansFromDegrees(lon2 - lon1);
	lat1 = RadiansFromDegrees(lat1);
	lat2 = RadiansFromDegrees(lat2);

	f64 a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
	f64 c = 2.0 * asin(sqrt(a));

	f64 Result = EarthRadius * c;

	return Result;
}

HaversineResult SumPairs(const std::vector<Pair>& pairs)
{
	TimeFunctionBandwidth(pairs.size() * sizeof(Pair));
	HaversineResult result;
	result.distances.resize(pairs.size());
	f64 sum = 0.0;
	for (u64 i = 0; i < pairs.size(); i++)
	{
		result.distances[i] = ReferenceHaversine(pairs[i], radius);
		sum += result.distances[i];
	}

	result.average = sum / (f64)pairs.size();
	return result;
}

void TestReadFunctions(const char* filename)
{
	auto filepath = std::filesystem::current_path().append(filename);
	auto fileSizeBytes = std::filesystem::file_size(filepath);
	std::vector<char> buffer(fileSizeBytes);

	RepetitionTester freadTester{ "fread", 10.0, fileSizeBytes, true};
	while (freadTester.IsTesting())
	{
		FILE* file = fopen(filename, "rb");
			
		freadTester.StartRun();

		fread(buffer.data(), sizeof(buffer[0]), sizeof(buffer[0]) * buffer.size(), file);

		freadTester.EndRun();

		fclose(file);
	}
	freadTester.PrintDiagnostics();

	RepetitionTester ifstreamTester{ "ifstream", 10.0, fileSizeBytes, true };
	while (ifstreamTester.IsTesting())
	{
		char* bufferPtr = &buffer[0];
		auto bytesLeft = buffer.size();
		std::ifstream file(filename, std::ios::binary);
		file.rdbuf()->pubsetbuf(&buffer[0], buffer.size());

		ifstreamTester.StartRun();

		while (bytesLeft > 0)
		{
			constexpr auto maxBytesToRead = std::numeric_limits<std::streamsize>::max();
			auto bytesToRead = bytesLeft < maxBytesToRead ? bytesLeft : maxBytesToRead;
			file.read(&buffer[0], buffer.size());
			assert(file);
			bytesLeft -= bytesToRead;
		}

		file.read(&buffer[0], buffer.size());

		ifstreamTester.EndRun();
	}
	ifstreamTester.PrintDiagnostics();

	RepetitionTester readFileTester{ "ReadFile", 10.0, fileSizeBytes, true };
	while (readFileTester.IsTesting())
	{
		char* bufferPtr = &buffer[0];
		auto bytesLeft = buffer.size();
		HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		readFileTester.StartRun();

		while (bytesLeft > 0)
		{
			DWORD bytesRead;
			DWORD bytesToRead = bytesLeft < MAXDWORD ? bytesLeft : MAXDWORD;
			BOOL success = ReadFile(file, bufferPtr, bytesToRead, &bytesRead, NULL);
			assert(success);
			bytesLeft -= bytesRead;
			bufferPtr += bytesRead;
		}

		readFileTester.EndRun();
	}
	readFileTester.PrintDiagnostics();
}
