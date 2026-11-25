#pragma once

#include <Neoeter/Script/Script.h>

#include <string>

enum class ScriptCallStatus
{
	Ok,
	// The object has no callable attribute of that name.
	NoMethod,
	// The call raised; the backend already reported the exception.
	Error,
};

// A scripting runtime (Python, .NET, ...) that owns ScriptType::Object values.
//
// Wrapper lifetime contract: a backend keeps at most one wrapper per live
// foreign object (so pointer identity is stable) and the wrapper holds a
// *borrowed* reference to it, mirroring the borrowed PyObject* handlers the
// engine stores. Script_INCREF/DECREF on a wrapper forward to IncRef/DecRef,
// which take/drop a strong reference to the foreign object.
class ScriptBackend
{
	public:
		virtual ~ScriptBackend() = default;

		virtual const char* Name() const = 0;

		virtual void IncRef(ScriptObject* object) = 0;
		virtual void DecRef(ScriptObject* object) = 0;

		virtual bool HasAttr(ScriptObject* object, const char* name) = 0;
		// args is a borrowed tuple; *result receives a new reference on Ok.
		virtual ScriptCallStatus CallMethod(ScriptObject* object, const char* name, ScriptObject* args, ScriptObject** result) = 0;
		virtual std::string Repr(ScriptObject* object) = 0;

		// Optional services of the runtime that runs the game scripts.
		// RunFile executes a script file from the packs (app.RunPythonFile);
		// Update is called once per frame by the main loop.
		virtual bool RunFile(const char* path)
		{
			(void)path;
			return false;
		}
		virtual void Update() {}
};

// The backend that runs the game scripts (Python or .NET), so engine code can
// stay independent of the scripting runtime. nullptr when none is active.
void Script_SetActiveBackend(ScriptBackend* backend);
ScriptBackend* Script_GetActiveBackend();
bool Script_RunFile(const char* path);
void Script_UpdateActiveBackend();
