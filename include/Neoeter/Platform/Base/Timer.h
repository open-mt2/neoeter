#pragma once

#if (defined(_WIN32) || defined(_WIN64)) && !defined(USE_PLATFORM_COMPAT)
#include <windows.h>
#include <cstdint>
#include <cstddef>
#else
#include <Platform/Platform.h>
#endif
#include "Singleton.h"

class CTimer : public CSingleton<CTimer>
{
	public:
		CTimer();
		virtual ~CTimer();

		void Advance();
		void Adjust(int iTimeGap);
		void SetBaseTime();

		float GetCurrentSecond();
		uint32_t GetCurrentMillisecond();

		float GetElapsedSecond();
		uint32_t GetElapsedMilliecond();

		void UseCustomTime();

	protected:
		bool m_bUseRealTime;
		uint32_t m_dwBaseTime;
		uint32_t m_dwCurrentTime;
		float m_fCurrentTime;
		uint32_t m_dwElapsedTime;
		int m_index;
};

int32_t ELTimer_Init();

uint32_t ELTimer_GetMSec();

void ELTimer_SetServerMSec(uint32_t dwServerTime);
uint32_t ELTimer_GetServerMSec();

void ELTimer_SetFrameMSec();
uint32_t ELTimer_GetFrameMSec();