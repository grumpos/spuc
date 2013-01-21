
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;

struct Ticker
{
	std::vector<uint64_t> ticks_;
	uint64_t freq_;
	uint64_t t0_;

	Ticker() 
		: freq_(0),
		t0_(0)
	{
		ticks_.reserve(32);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq_);
		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
	}

	void Tick()
	{
		uint64_t t1 = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&t1);
		ticks_.push_back(t1 - t0_);
		t0_ = t1;
	}

	void Reset()
	{
		ticks_.clear();
		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
	}

	string Stats()
	{
		std::ostringstream oss;
		std::for_each( ticks_.cbegin(), ticks_.cend(), [&oss](uint64_t t){ oss << t << ", "; });
		return oss.str();
	}
};
