#pragma once
class Timer
{
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	long long unsigned m_count = 0;

public:
	void start_timer();

	void time();
};