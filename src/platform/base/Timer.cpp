#include <Neoeter/Platform/Base/StdAfx.h>
#include <Neoeter/Platform/Base/Timer.h>
#include <Neoeter/Platform/Port.h>

static uint32_t gs_dwBaseTime = 0;
static uint32_t gs_dwServerTime = 0;
static uint32_t gs_dwClientTime = 0;
static uint32_t gs_dwFrameTime = 0;

int32_t ELTimer_Init()
{
	gs_dwBaseTime = Neoeter::Platform::TickCount();
	return 1;
}

uint32_t ELTimer_GetMSec()
{
	return Neoeter::Platform::TickCount() - gs_dwBaseTime;
}

void ELTimer_SetServerMSec(uint32_t dwServerTime)
{
	gs_dwServerTime = dwServerTime;
	gs_dwClientTime = CTimer::instance().GetCurrentMillisecond();
}

uint32_t ELTimer_GetServerMSec()
{
	return CTimer::instance().GetCurrentMillisecond() - gs_dwClientTime + gs_dwServerTime;
}

uint32_t ELTimer_GetFrameMSec()
{
	return gs_dwFrameTime;
}

uint32_t ELTimer_GetServerFrameMSec()
{
	return ELTimer_GetFrameMSec() - gs_dwClientTime + gs_dwServerTime;
}

void ELTimer_SetFrameMSec()
{
	gs_dwFrameTime = ELTimer_GetMSec();
}

CTimer::CTimer()
{
	ELTimer_Init();

	m_dwCurrentTime = 0;
	m_bUseRealTime = true;
	m_index = 0;

	m_dwElapsedTime = 0;

	m_fCurrentTime = 0.0f;
}

CTimer::~CTimer()
{
}

void CTimer::SetBaseTime()
{
	m_dwCurrentTime = 0;
}

void CTimer::Advance()
{
	if (!m_bUseRealTime)
	{
		++m_index;

		if (m_index == 1)
			m_index = -1;

		m_dwCurrentTime += 16 + (m_index & 1);
		m_fCurrentTime = m_dwCurrentTime / 1000.0f;
	}
	else
	{
		uint32_t currentTime = ELTimer_GetMSec();

		if (m_dwCurrentTime == 0)
			m_dwCurrentTime = currentTime;

		m_dwElapsedTime = currentTime - m_dwCurrentTime;
		m_dwCurrentTime = currentTime;
	}
}

void CTimer::Adjust(int iTimeGap)
{
	m_dwCurrentTime += iTimeGap;
}

float CTimer::GetCurrentSecond()
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec() / 1000.0f;

	return m_fCurrentTime;
}

uint32_t CTimer::GetCurrentMillisecond()
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec();

	return m_dwCurrentTime;
}

float CTimer::GetElapsedSecond()
{
	return GetElapsedMilliecond() / 1000.0f;
}

uint32_t CTimer::GetElapsedMilliecond()
{
	if (!m_bUseRealTime)
		return 16 + (m_index & 1);

	return m_dwElapsedTime;
}

void CTimer::UseCustomTime()
{
	m_bUseRealTime = false;
}
