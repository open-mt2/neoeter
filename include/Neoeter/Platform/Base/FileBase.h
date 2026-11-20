#ifndef __INC_ETERBASE_FILEBASE_H__
#define __INC_ETERBASE_FILEBASE_H__

#if (defined(_WIN32) || defined(_WIN64)) && !defined(USE_PLATFORM_COMPAT)
#include <windows.h>
#include <cstdint>
#include <cstddef>
#else
#include <Neoeter/Platform/Platform.h>
#endif

class CFileBase
{
	public:
		enum EFileMode
		{
			FILEMODE_READ = (1 << 0),
			FILEMODE_WRITE = (1 << 1)
		};

		CFileBase();
		virtual ~CFileBase();

		void Destroy();
		void Close();

		int32_t Create(const char* filename, EFileMode mode);
		uint32_t Size();
		void SeekCur(uint32_t size);
		void Seek(uint32_t offset);
		uint32_t GetPosition();

		virtual int32_t Write(const void* src, int bytes);
		int32_t Read(void* dest, int bytes);

		char* GetFileName();
		int32_t IsNull();

	protected:
		int m_mode;
		char m_filename[MAX_PATH + 1];
		HANDLE m_hFile;
		uint32_t m_dwSize;
};

#endif
