#pragma once

#include <string>

// Collects the text of a script exception (traceback, dbg.RegisterExceptionString)
// so the client can forward it, e.g. to the game server. Every backend calls
// Clear() before reporting an exception and Send() afterwards.
class IScriptExceptionSender
{
	public:
		virtual ~IScriptExceptionSender() = default;

		void Clear()
		{
			m_strExceptionString.clear();
		}

		void RegisterExceptionString(const char* c_szString)
		{
			m_strExceptionString += c_szString;
		}

		virtual void Send() = 0;

	protected:
		std::string m_strExceptionString;
};

// Historic name, kept for out-of-tree code.
using IPythonExceptionSender = IScriptExceptionSender;

extern IScriptExceptionSender* g_pkExceptionSender;

void SetExceptionSender(IScriptExceptionSender* pkExceptionSender);

// Reports an exception through the registered sender (if any).
void Script_ReportException(const char* text);
