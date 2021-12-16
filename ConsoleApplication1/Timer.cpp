#include "pch.h"
#include "Timer.h"

void Timer::time()
{
	if (m_count++ == 0)
	{
		return start_timer();
	}
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	wchar_t text_buffer[200] = { 0 }; //temporary buffer
	swprintf(text_buffer, _countof(text_buffer), L"Microseconds since last: %lld\n", ElapsedMicroseconds.QuadPart); // convert
	OutputDebugString(text_buffer); // print

	StartingTime = EndingTime;

}
void Timer::start_timer()
{
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);
}