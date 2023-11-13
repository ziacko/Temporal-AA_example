#ifndef TINYCLOCK_H
#define TINYCLOCK_H

#if defined (_WIN32) || defined (_WIN64)
#define TC_WINDOWS
#include <Windows.h>
#elif defined (__linux__)
#define TC_LINUX
#include <sys/time.h>
#include <stdint.h>
#endif

#include <time.h>
#include <stdio.h>
#include <math.h>
#include <thread>
#include <chrono>

class tinyClock_t
{
public:

	tinyClock_t(void)
	{
		Initialize();
	}
	~tinyClock_t(void)
	{

	}

	/**
	 * Initialize the TinyClock API
	 */
	void Initialize(void)
	{
		totalTime = 0;
		deltaTime = 0;

#if defined (TC_WINDOWS)
		Windows_Initialize();
#elif defined (TC_LINUX)
		Linux_Initialize();
#endif
	}
	/**
	 * update the clock using a fixed time step. e.g 60
	 */
	inline void UpdateClockFixed(double TimeStep, bool forceFrameRate = true)
	{
		deltaTime = 1.0 / TimeStep;
		double NewTime = tinyClock_t::GetTime();
		totalTime = NewTime;
		if (1.0 / TimeStep > deltaTime && forceFrameRate)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1.0 / TimeStep) * 1000));// static_cast<long>(1.0 / 60.0 - (sceneClock->GetDeltaTime() * 1000.0))));
			printf("%f \n", (1.0 / 60.0 - deltaTime * 1000));
		}
	}

	/**
	 * update the clock using adaptive CPU clocking
	 */
	inline void UpdateClockAdaptive(void)
	{
		double NewTime = tinyClock_t::GetTime();
		deltaTime = NewTime - totalTime;
		totalTime = NewTime;
	}	
	/**
	 * get the total amount of time TinyClock has been running in milliseconds
	 */
	double GetTotalTime(void)
	{
		return totalTime;
	}
	/**
	 * get the current CPU delta time (time between CPU cycles in milliseconds)
	 */
	double GetDeltaTime(void)
	{
		return deltaTime;
	}

private:

	const double windowsLow = 0.001;
	const double floatLow = 1e-6;
	const double floatHigh = 1e-9;
	const double linuxHigh = 1e9;
	const double linuxLow = 1e6;
	const double millisecond = 1000.0;
	const double microsecond = 1000000.0;

	/**
	 * get the Time of the computer
	 */
	double GetTime(void)
	{
#if defined (TC_WINDOWS)
		return Windows_GetTime();
#elif defined (TC_LINUX)
		return Linux_GetTime();
#endif
	}
	/**
	 * get the amount of time the computer has been running
	 */
	double GetRawTime(void)
	{
#if defined (TC_WINDOWS)
			return (double)Windows_GetRawTime();
#elif defined (TC_LINUX) 
			return (double)Linux_GetRawTime();
#endif
	}

#if defined (TC_WINDOWS)

	/**
	 * Initialize tinyClock on the Windows platform
	 */
	void Windows_Initialize(void)
	{
		unsigned __int64 Frequency;

		//the frequency of the performance counter in seconds
		if (QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency))
		{
			/*
			 * if QueryPerformanceFrequency does not return 0 then your computer supports
			 * high resolution Time steps.
			 */
			supportsHighRes = true;
			timeResolution = 1.0 / (double)Frequency;
		}

		else
		{
			supportsHighRes = false;
			timeResolution = windowsLow;
		}

		//base time is your computers time when tinyWindow initializes
		baseTime = (double)Windows_GetRawTime();
	}
	/**
	 * get the amount of time since the system was turned on
	 */
	unsigned __int64 Windows_GetRawTime(void)
	{
			if (supportsHighRes)
			{
				unsigned __int64 Time = 0;
				//the current value of the performance counter in counts for high resolution time counts
				QueryPerformanceCounter((LARGE_INTEGER*)&Time);
				return Time;
			}

			//the amount of milliseconds since the system was started
			return (unsigned __int64)GetTickCount();
	}
	/**
	 * gets the current raw time of the computer and subtracts the value using the raw
	 * time that was collected when TinyClock was Initialized. then multiply that value
	 * by the current Resolution.
	 */
	double Windows_GetTime(void)
	{
			return (double)(Windows_GetRawTime() - baseTime) * timeResolution;
	}

#elif defined (TC_LINUX)

	/*
	 * Initialize TinyClock on the Linux platform
	 */
	void Linux_Initialize(void)
	{
		monoticSupported = false;

		//If monotonic is supported. Monotonic time since an unspecified starting point 
#if defined (CLOCK_MONOTONIC)
		struct timespec ts;

		if (!clock_gettime(CLOCK_MONOTONIC, &ts))
		{
			monoticSupported = true;
			timeResolution = floatHigh;
		}
		else
#endif
		{
			timeResolution = floatLow;
		}

		baseTime = GetRawTime();
	}
	/**
	 * get the amount of time since the system was turned on in milliseconds
	 */
	uint64_t Linux_GetRawTime(void)
	{
#if defined (CLOCK_MONOTONIC)
		if (monoticSupported)
		{
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			return (uint64_t)ts.tv_sec * (uint64_t)linuxHigh + (uint64_t)ts.tv_nsec;
		}

		else
#endif
		{
			struct timeval TimeVal;
			gettimeofday(&TimeVal, 0);
			return (uint64_t)TimeVal.tv_sec * (uint64_t)linuxHigh + (uint64_t)TimeVal.tv_usec;
		}
	}
	/**
	 * gets the current raw time of the computer and subtracts the value using the raw
	 * time that was collected when TinyClock was Initialized. then multiply that value
	 * by the current Resolution.
	 */
	double Linux_GetTime()
	{
		return (double)(GetRawTime() - baseTime) * timeResolution;
	}

#endif

	double					totalTime;				/**< The total amount of time since TinyClock was initialized */
	double					deltaTime;				/**< The delta time. The amount of time between CPU cycles */
	double					timeResolution;			/**< The resolution of the time step. (seconds, milliseconds, etc.) */
	double					baseTime;				/**< The system time since TinyClock was initialize */

#if defined (TC_WINDOWS)
	bool					supportsHighRes;		/**< Whether high resolution time step is supported */
#elif defined (TC_LINUX)
	bool					monoticSupported;		/**< Is monotonic supported */
#endif
};
#endif
