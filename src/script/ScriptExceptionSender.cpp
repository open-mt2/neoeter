#include <Neoeter/Script/ScriptExceptionSender.h>

IScriptExceptionSender* g_pkExceptionSender = nullptr;

void SetExceptionSender(IScriptExceptionSender* pkExceptionSender)
{
	g_pkExceptionSender = pkExceptionSender;
}

void Script_ReportException(const char* text)
{
	if (!g_pkExceptionSender)
		return;

	g_pkExceptionSender->Clear();
	g_pkExceptionSender->RegisterExceptionString(text ? text : "");
	g_pkExceptionSender->Send();
}
