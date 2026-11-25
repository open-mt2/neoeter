#include "ScriptInternal.h"

#include <Neoeter/Script/ScriptBackend.h>

#include <cstdio>

ScriptObject* Script_BadArgument()
{
	ScriptErr_SetString(ScriptErrorType::TypeError, "bad argument type for built-in operation");
	return nullptr;
}

ScriptObject* Script_BuildException(const char* format, ...)
{
	if (!format)
		ScriptErr_Clear();
	else
	{
		char buffer[512 + 1];
		va_list args;
		va_start(args, format);
		std::vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		ScriptErr_SetString(ScriptErrorType::RuntimeError, buffer);
	}

	return Script_BuildNone();
}

ScriptObject* Script_BuildNone()
{
	return Script_None();
}

ScriptObject* Script_BuildEmptyTuple()
{
	return ScriptTuple_New(0);
}

ScriptObject* Script_BuildHandle(const void* handle)
{
	return ScriptInt_New(static_cast<std::int64_t>(reinterpret_cast<std::intptr_t>(handle)));
}

static ScriptObject* TupleItem(ScriptObject* args, int pos)
{
	if (!args || pos < 0 || pos >= ScriptTuple_Size(args))
		return nullptr;

	return ScriptTuple_GetItem(args, pos);
}

bool ScriptTuple_GetObject(ScriptObject* args, int pos, ScriptObject** ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = item;
	return true;
}

bool ScriptTuple_GetLong(ScriptObject* args, int pos, long* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = static_cast<long>(Script_AsLongLong(item));
	return true;
}

bool ScriptTuple_GetHandle(ScriptObject* args, int pos, void** handle)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item || !ScriptInt_Check(item))
		return false;

	*handle = reinterpret_cast<void*>(static_cast<std::intptr_t>(Script_AsLongLong(item)));
	return true;
}

bool ScriptTuple_GetDouble(ScriptObject* args, int pos, double* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = Script_AsDouble(item);
	return true;
}

bool ScriptTuple_GetFloat(ScriptObject* args, int pos, float* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = static_cast<float>(Script_AsDouble(item));
	return true;
}

bool ScriptTuple_GetInteger(ScriptObject* args, int pos, int* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = static_cast<int>(Script_AsLongLong(item));
	return true;
}

// The narrow overloads keep the historic behaviour of writing the truncated
// value even when the lookup fails.
bool ScriptTuple_GetByte(ScriptObject* args, int pos, unsigned char* ret)
{
	int value = 0;
	const bool result = ScriptTuple_GetInteger(args, pos, &value);
	*ret = static_cast<unsigned char>(value);
	return result;
}

bool ScriptTuple_GetInteger(ScriptObject* args, int pos, unsigned char* ret)
{
	return ScriptTuple_GetByte(args, pos, ret);
}

bool ScriptTuple_GetInteger(ScriptObject* args, int pos, unsigned short* ret)
{
	int value = 0;
	const bool result = ScriptTuple_GetInteger(args, pos, &value);
	*ret = static_cast<unsigned short>(value);
	return result;
}

bool ScriptTuple_GetUnsignedLong(ScriptObject* args, int pos, unsigned long* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = static_cast<unsigned long>(Script_AsUnsignedLongLong(item));
	return true;
}

bool ScriptTuple_GetUnsignedInteger(ScriptObject* args, int pos, unsigned int* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = static_cast<unsigned int>(Script_AsUnsignedLongLong(item));
	return true;
}

bool ScriptTuple_GetString(ScriptObject* args, int pos, char** ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item || !ScriptString_Check(item))
		return false;

	*ret = ScriptString_AsString(item);
	return true;
}

bool ScriptTuple_GetBoolean(ScriptObject* args, int pos, bool* ret)
{
	ScriptObject* item = TupleItem(args, pos);
	if (!item)
		return false;

	*ret = Script_AsLongLong(item) != 0;
	return true;
}

// ---------------------------------------------------------------------------
// Calls into script objects
// ---------------------------------------------------------------------------
ScriptObject* Script_CallMethodObject(ScriptObject* object, const char* name, ScriptObject* args)
{
	if (!object || !ScriptForeign_Check(object) || !object->Backend() || !object->Handle())
	{
		Script_XDECREF(args);
		return nullptr;
	}

	ScriptObject* callArgs = args ? args : ScriptTuple_New(0);
	ScriptObject* result = nullptr;
	const ScriptCallStatus status = object->Backend()->CallMethod(object, name, callArgs, &result);
	Script_DecRef(callArgs);

	return status == ScriptCallStatus::Ok ? result : nullptr;
}

bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args)
{
	ScriptObject* result = Script_CallMethodObject(object, name, args);
	if (!result)
		return false;

	Script_DecRef(result);
	return true;
}

bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args, bool* ret)
{
	ScriptObject* result = Script_CallMethodObject(object, name, args);
	if (!result)
		return false;

	*ret = ScriptNumber_Check(result) ? Script_AsLongLong(result) != 0 : true;
	Script_DecRef(result);
	return true;
}

bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args, long* ret)
{
	ScriptObject* result = Script_CallMethodObject(object, name, args);
	if (!result)
		return false;

	const bool isNumber = ScriptNumber_Check(result);
	if (isNumber)
		*ret = static_cast<long>(Script_AsLongLong(result));

	Script_DecRef(result);
	return isNumber;
}

bool Script_HasAttr(ScriptObject* object, const char* name)
{
	if (!object || !ScriptForeign_Check(object) || !object->Backend() || !object->Handle())
		return false;

	return object->Backend()->HasAttr(object, name);
}

namespace
{
	ScriptBackend* g_activeBackend = nullptr;
}

void Script_SetActiveBackend(ScriptBackend* backend)
{
	g_activeBackend = backend;
}

ScriptBackend* Script_GetActiveBackend()
{
	return g_activeBackend;
}

bool Script_RunFile(const char* path)
{
	return g_activeBackend && path && g_activeBackend->RunFile(path);
}

void Script_UpdateActiveBackend()
{
	if (g_activeBackend)
		g_activeBackend->Update();
}
