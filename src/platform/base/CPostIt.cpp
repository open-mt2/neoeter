#include <Neoeter/Platform/Base/StdAfx.h>

#include <Neoeter/Platform/Base/CPostIt.h>
#include <Neoeter/Platform/Base/Utils.h>

class _CPostItMemoryBlock
{
	public:
		_CPostItMemoryBlock(void);
		~_CPostItMemoryBlock(void);

		int32_t Assign(HANDLE hBlock);
		HANDLE CreateHandle(void);
		int32_t DestroyHandle(void);

		LPSTR Find(LPCSTR lpszKeyName);
		int32_t Put(LPCSTR lpBuffer);
		int32_t Put(LPCSTR lpszKeyName, LPCSTR lpBuffer);
		int32_t Get(LPCSTR lpszKeyName, LPSTR lpBuffer, uint32_t nSize);

	protected:
		typedef std::list<char*> StrList;
		typedef StrList::iterator StrListItr;

		HANDLE m_hHandle;
		StrList m_StrList;
};

CPostIt::CPostIt(LPCSTR szAppName) : m_bModified(FALSE), m_pMemoryBlock(NULL)
{
	Init(szAppName);
}

CPostIt::~CPostIt(void) { Destroy(); }

int32_t CPostIt::Init(LPCSTR szAppName)
{
	if (szAppName == NULL || !*szAppName)
	{
		strcpy(m_szClipFormatName, "YMCF");
	}
	else
	{
		strcpy(m_szClipFormatName, "YMCF_");
		strcat(m_szClipFormatName, szAppName);
	}
	return TRUE;
}

int32_t CPostIt::CopyTo(CPostIt* pPostIt, LPCSTR lpszKeyName)
{
	if (m_pMemoryBlock == NULL)
		return FALSE;
	LPSTR szText = m_pMemoryBlock->Find(lpszKeyName);
	if (szText == NULL)
		return FALSE;
	return pPostIt->Set(szText);
}

int32_t CPostIt::Flush(void)
{
	if (m_bModified == FALSE)
		return FALSE;
	if (m_pMemoryBlock == NULL)
		return FALSE;

#if defined(_WIN32)
	uint32_t uDGPFormat;

	uDGPFormat = ::RegisterClipboardFormatA(m_szClipFormatName);
	if (::OpenClipboard(NULL) == FALSE)
		return FALSE;
	if (::SetClipboardData(uDGPFormat, m_pMemoryBlock->CreateHandle()) == NULL)
	{
		//		uint32_t dwLastError = ::GetLastError();
		m_pMemoryBlock->DestroyHandle();
		::CloseClipboard();
		m_bModified = FALSE;
		return FALSE;
	}
	::CloseClipboard();
#endif

	m_bModified = FALSE;
	return TRUE;
}

void CPostIt::Empty(void)
{
	SAFE_DELETE(m_pMemoryBlock);

#if defined(_WIN32)
	uint32_t uDGPFormat;

	uDGPFormat = ::RegisterClipboardFormatA(m_szClipFormatName);
	if (::OpenClipboard(NULL) == FALSE)
		return;
	HANDLE hClipboardMemory = ::GetClipboardData(uDGPFormat);
	if (hClipboardMemory)
	{
		//		::GlobalFree( hClipboardMemory );
		::SetClipboardData(uDGPFormat, NULL);
	}
	::CloseClipboard();
#endif

	m_bModified = FALSE;
}

void CPostIt::Destroy(void)
{
	Flush();
	SAFE_DELETE(m_pMemoryBlock);
}

int32_t CPostIt::Set(LPCSTR lpszKeyName, LPCSTR lpBuffer)
{
	if (m_pMemoryBlock == NULL)
		m_pMemoryBlock = new _CPostItMemoryBlock;
	m_pMemoryBlock->Put(lpszKeyName, lpBuffer);
	m_bModified = TRUE;
	return TRUE;
}

int32_t CPostIt::Set(LPCSTR lpszKeyName, uint32_t dwValue)
{
	char szValue[12];

	_snprintf(szValue, 12, "%d", dwValue);
	return Set(lpszKeyName, szValue);
}

int32_t CPostIt::Set(LPCSTR lpBuffer)
{
	if (lpBuffer == NULL)
		return FALSE;
	if (m_pMemoryBlock == NULL)
		m_pMemoryBlock = new _CPostItMemoryBlock;
	m_pMemoryBlock->Put(lpBuffer);
	m_bModified = TRUE;
	return TRUE;
}

int32_t CPostIt::Get(LPCSTR lpszKeyName, LPSTR lpBuffer, uint32_t nSize)
{
	if (m_pMemoryBlock == NULL)
	{
#if defined(_WIN32)
		uint32_t uDGPFormat;

		uDGPFormat = ::RegisterClipboardFormatA(m_szClipFormatName);
		if (::OpenClipboard(NULL) == FALSE)
			return FALSE;

		HANDLE hClipboardMemory = ::GetClipboardData(uDGPFormat);

		if (hClipboardMemory == NULL)
		{
			::CloseClipboard();
			return FALSE;
		}
		m_pMemoryBlock = new _CPostItMemoryBlock;
		m_pMemoryBlock->Assign(hClipboardMemory);

		::CloseClipboard();
#else
		return FALSE;
#endif
	}

	return m_pMemoryBlock->Get(lpszKeyName, lpBuffer, nSize);
}

_CPostItMemoryBlock::_CPostItMemoryBlock(void) : m_hHandle(NULL) {}

_CPostItMemoryBlock::~_CPostItMemoryBlock(void)
{
	for (StrListItr itr = m_StrList.begin(); itr != m_StrList.end();)
	{
		LPSTR lpszText = *itr;
		SAFE_DELETE_ARRAY(lpszText);
		itr = m_StrList.erase(itr);
	}
}

int32_t _CPostItMemoryBlock::Assign(HANDLE hBlock)
{
	if (hBlock == NULL || hBlock == INVALID_HANDLE_VALUE)
		return FALSE;
	LPBYTE lpBuffer = (LPBYTE)::GlobalLock(hBlock);

	if (lpBuffer == NULL)
		return FALSE;
	uint32_t dwCount = *((LPDWORD)lpBuffer);
	lpBuffer += sizeof(uint32_t);
	for (uint32_t dwI = 0; dwI < dwCount; dwI++)
	{
		uint16_t wLen = *((LPWORD)lpBuffer);
		lpBuffer += sizeof(uint16_t);

		LPSTR lpszText = new char[wLen + 1];
		::CopyMemory(lpszText, lpBuffer, wLen);
		lpszText[wLen] = '\0';

		lpBuffer += wLen;

		Put(lpszText);
	}
	::GlobalUnlock(hBlock);
	return TRUE;
}

HANDLE _CPostItMemoryBlock::CreateHandle(void)
{
	if (m_StrList.size() == 0)
		return INVALID_HANDLE_VALUE;
	uint32_t dwBlockSize = sizeof(uint32_t);
	StrListItr itr;

	// Calculation for Memory Block Size
	for (itr = m_StrList.begin(); itr != m_StrList.end(); ++itr)
	{
		dwBlockSize += sizeof(uint16_t);
		dwBlockSize += (uint32_t)strlen(*itr);
	}

	HANDLE hBlock = ::GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, dwBlockSize);
	if (hBlock == NULL)
		return INVALID_HANDLE_VALUE;
	LPBYTE lpBuffer = (LPBYTE)::GlobalLock(hBlock);
	if (lpBuffer == NULL)
	{
		::GlobalFree(hBlock);
		return INVALID_HANDLE_VALUE;
	}
	*((LPDWORD)lpBuffer) = (uint32_t)m_StrList.size();
	lpBuffer += sizeof(uint32_t);
	for (itr = m_StrList.begin(); itr != m_StrList.end(); ++itr)
	{
		*((LPWORD)lpBuffer) = (uint16_t)strlen(*itr);
		lpBuffer += sizeof(uint16_t);
		::CopyMemory(lpBuffer, *itr, strlen(*itr));
		lpBuffer += strlen(*itr);
	}
	::GlobalUnlock(hBlock);

	m_hHandle = hBlock;
	return hBlock;
}

int32_t _CPostItMemoryBlock::DestroyHandle(void)
{
	::GlobalFree(m_hHandle);
	m_hHandle = NULL;
	return TRUE;
}

LPSTR _CPostItMemoryBlock::Find(LPCSTR lpszKeyName)
{
	for (StrListItr itr = m_StrList.begin(); itr != m_StrList.end(); ++itr)
	{
		LPSTR lpszText = *itr;

		if (_strnicmp(lpszText, lpszKeyName, strlen(lpszKeyName)) != 0)
			continue;
		if (*(lpszText + strlen(lpszKeyName)) != '=')
			continue;
		return lpszText;
	}
	return NULL;
}

int32_t _CPostItMemoryBlock::Put(LPCSTR lpszKeyName, LPCSTR lpBuffer)
{
	LPSTR lpszText;

	if ((lpszText = Find(lpszKeyName)) != NULL)
	{
		for (StrListItr itr = m_StrList.begin(); itr != m_StrList.end(); ++itr)
		{
			if (lpszText == *itr)
			{
				SAFE_DELETE_ARRAY(lpszText);
				m_StrList.erase(itr);
				break;
			}
		}
	}
	if (lpBuffer == NULL || !*lpBuffer)
		return TRUE;
	size_t nStrLen = strlen(lpszKeyName) + 1 /* '=' */ + strlen(lpBuffer);
	lpszText = new char[nStrLen + 1];
	::CopyMemory(lpszText, lpszKeyName, strlen(lpszKeyName));
	*(lpszText + strlen(lpszKeyName)) = '=';
	::CopyMemory(lpszText + strlen(lpszKeyName) + 1, lpBuffer, strlen(lpBuffer));
	*(lpszText + nStrLen) = '\0';

	m_StrList.push_back(lpszText);
	return TRUE;
}

int32_t _CPostItMemoryBlock::Put(LPCSTR lpBuffer)
{
	LPSTR lpszText;

	if (lpBuffer == NULL || !*lpBuffer)
		return TRUE;
	size_t nStrLen = strlen(lpBuffer);
	lpszText = new char[nStrLen + 1];
	::CopyMemory(lpszText, lpBuffer, nStrLen);
	*(lpszText + nStrLen) = '\0';

	m_StrList.push_back(lpszText);
	return TRUE;
}

int32_t _CPostItMemoryBlock::Get(LPCSTR lpszKeyName, LPSTR lpBuffer, uint32_t nSize)
{
	LPSTR lpszText = Find(lpszKeyName);
	if (lpszText == NULL)
		return FALSE;
	lpszText += (strlen(lpszKeyName) + 1);
	::ZeroMemory(lpBuffer, nSize);
	strncpy(lpBuffer, lpszText,
			(nSize < strlen(lpszText)) ? nSize : strlen(lpszText));
	return TRUE;
}
