#ifndef _EL_CPOSTIT_H_
#define _EL_CPOSTIT_H_

#if (defined(_WIN32) || defined(_WIN64)) && !defined(USE_PLATFORM_COMPAT)
#include <windows.h>
#include <cstdint>
#include <cstddef>
#else
#include <Platform/Platform.h>
#endif

// _CPostItMemoryBlock is defined in CPostIt.cpp
class _CPostItMemoryBlock;

/**
 * @class	CPostIt
 * @brief	게임런처에서 게임 클라이언트로 정보를 전달 및 클라이언트에서
 * 수신하기 위하여 사용되는 클래스
 */
class CPostIt
{
	public:
		/**
		 * @brief	CPostIt constructor
		 * @param [in]	szAppName	: 게임의 이름이 들어간다.
		 */
		explicit CPostIt(LPCSTR szAppName);

		/**
		 * @brief	CPostIt destructor
		 */
		~CPostIt(void);

		/**
		 * @brief	CPostIt class에서 보유하고 있는 데이타를 클립보드에 저장한다.
		 */
		int32_t Flush(void);

		/**
		 * @brief	CPostIt class에서 보유하고 있는 데이타 및 클립보드에 있는 내용을
		 * 지운다.
		 */
		void Empty(void);

		/**
		 * @brief	데이타를 읽어온다.
		 * @param [in]	lpszKeyName	: 불러올 데이타의 키. "KEY" 식의 내용을
		 * 넣는다.
		 * @param [in]	lpszData	: 불러올 데이타의 버퍼
		 * @param [in]	nSize		: lpszData 버퍼의 최대사이즈
		 */
		int32_t Get(LPCSTR lpszKeyName, LPSTR lpszData, uint32_t nSize);

		/**
		 * @brief	저장할 데이타를 넣는다.
		 * @param [in]	lpBuffer	: 저장할 데이타. "KEY=DATA" 식의 내용을
		 * 넣는다.
		 */
		int32_t Set(LPCSTR lpszData);

		/**
		 * @brief	저장할 데이타를 넣는다.
		 * @param [in]	lpszKeyName	: 저장할 데이타의 키. "KEY" 식의 내용을
		 * 넣는다.
		 * @param [in]	lpszData	: 저장할 데이타. "DATA" 식의 내용을
		 * 넣는다.
		 */
		int32_t Set(LPCSTR lpszKeyName, LPCSTR lpszData);

		/**
		 * @brief	저장할 데이타(uint32_t)를 넣는다.
		 * @param [in]	lpBuffer	: 저장할 데이타. "KEY=DATA" 식의
		 * 데이타를 넣는다.
		 * @param [in]	dwValue		: 저장할 데이타. (uint32_t)
		 */
		int32_t Set(LPCSTR lpszKeyName, uint32_t dwValue);

		/**
		 * @brief	CPostIt class를 복사한다. (클래스 constructor에 이름 인자가 있기
		 * 때문에, 새 이름을 지정해야함)
		 * @param [in]	pPostIt		: Destination class
		 * @param [in]	lpszKeyName	: Destination class's new app-name
		 */
		int32_t CopyTo(CPostIt* pPostIt, LPCSTR lpszKeyName);

	protected:
		int32_t Init(LPCSTR szAppName);
		void Destroy(void);

	protected:
		int32_t m_bModified;
		char m_szClipFormatName[_MAX_PATH];
		_CPostItMemoryBlock* m_pMemoryBlock;
};

#endif /* _EL_CPOSTIT_H_ */
