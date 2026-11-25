#include "ScriptInternal.h"

#include <Neoeter/Script/ScriptBackend.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>

ScriptObject* ScriptObjectAccess::Make(ScriptType type)
{
	return new ScriptObject(type);
}

void ScriptObjectAccess::Destroy(ScriptObject* object)
{
	for (ScriptObject* item : object->m_items)
		Script_DecRef(item);

	for (auto& entry : object->m_entries)
	{
		Script_DecRef(entry.first);
		Script_DecRef(entry.second);
	}

	delete object;
}

// ---------------------------------------------------------------------------
// Reference counting
// ---------------------------------------------------------------------------
void Script_IncRef(ScriptObject* object)
{
	switch (object->Type())
	{
		case ScriptType::None:
		case ScriptType::Module:
			return;
		case ScriptType::Object:
			if (object->Backend())
				object->Backend()->IncRef(object);
			return;
		default:
			++ScriptObjectAccess::RefCount(object);
	}
}

void Script_DecRef(ScriptObject* object)
{
	switch (object->Type())
	{
		case ScriptType::None:
		case ScriptType::Module:
			return;
		case ScriptType::Object:
			if (object->Backend())
				object->Backend()->DecRef(object);
			return;
		default:
			if (--ScriptObjectAccess::RefCount(object) == 0)
				ScriptObjectAccess::Destroy(object);
	}
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
ScriptObject* Script_None()
{
	static ScriptObject* s_none = ScriptObjectAccess::Make(ScriptType::None);
	return s_none;
}

ScriptObject* ScriptBool_New(bool value)
{
	ScriptObject* object = ScriptObjectAccess::Make(ScriptType::Bool);
	ScriptObjectAccess::Value(object).b = value;
	return object;
}

ScriptObject* ScriptInt_New(std::int64_t value)
{
	ScriptObject* object = ScriptObjectAccess::Make(ScriptType::Int);
	ScriptObjectAccess::Value(object).i = value;
	return object;
}

ScriptObject* ScriptFloat_New(double value)
{
	ScriptObject* object = ScriptObjectAccess::Make(ScriptType::Float);
	ScriptObjectAccess::Value(object).f = value;
	return object;
}

ScriptObject* ScriptString_New(const char* value)
{
	return ScriptString_NewSize(value ? value : "", value ? std::strlen(value) : 0);
}

ScriptObject* ScriptString_NewSize(const char* data, std::size_t size)
{
	ScriptObject* object = ScriptObjectAccess::Make(ScriptType::String);
	if (data && size)
		object->MutableBytes().assign(data, size);
	else
		object->MutableBytes().assign(size, '\0');
	return object;
}

static ScriptObject* NewSequence(ScriptType type, std::size_t size)
{
	ScriptObject* object = ScriptObjectAccess::Make(type);
	ScriptObjectAccess::Items(object).assign(size, Script_None());
	return object;
}

ScriptObject* ScriptTuple_New(std::size_t size)
{
	return NewSequence(ScriptType::Tuple, size);
}

ScriptObject* ScriptList_New(std::size_t size)
{
	return NewSequence(ScriptType::List, size);
}

ScriptObject* ScriptDict_New()
{
	return ScriptObjectAccess::Make(ScriptType::Dict);
}

ScriptObject* ScriptObject_NewForeign(ScriptBackend* backend, void* handle)
{
	ScriptObject* object = ScriptObjectAccess::Make(ScriptType::Object);
	ScriptObjectAccess::Backend(object) = backend;
	object->SetHandle(handle);
	return object;
}

void ScriptObject_FreeForeign(ScriptObject* object)
{
	if (object && object->Type() == ScriptType::Object)
		ScriptObjectAccess::Destroy(object);
}

ScriptObject* ScriptObjectAccess::MakeModule(ScriptModuleData* module)
{
	ScriptObject* object = Make(ScriptType::Module);
	ScriptObjectAccess::Module(object) = module;
	return object;
}

// ---------------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------------
std::int64_t Script_AsLongLong(ScriptObject* object)
{
	if (object)
	{
		switch (object->Type())
		{
			case ScriptType::Int:
				return object->AsInt();
			case ScriptType::Bool:
				return object->AsBool() ? 1 : 0;
			case ScriptType::Float:
			{
				const double value = object->AsFloat();
				if (std::isnan(value) || value >= 9.2233720368547758e18 || value < -9.2233720368547758e18)
				{
					ScriptErr_SetString(ScriptErrorType::OverflowError, "float too large to convert to integer");
					return -1;
				}
				return static_cast<std::int64_t>(value);
			}
			default:
				break;
		}
	}

	ScriptErr_SetString(ScriptErrorType::TypeError, "an integer is required");
	return -1;
}

std::uint64_t Script_AsUnsignedLongLong(ScriptObject* object)
{
	const std::int64_t value = Script_AsLongLong(object);
	if (value < 0 && ScriptNumber_Check(object))
	{
		ScriptErr_SetString(ScriptErrorType::OverflowError, "can't convert negative value to unsigned long");
		return static_cast<std::uint64_t>(-1);
	}

	return static_cast<std::uint64_t>(value);
}

double Script_AsDouble(ScriptObject* object)
{
	if (object)
	{
		switch (object->Type())
		{
			case ScriptType::Float:
				return object->AsFloat();
			case ScriptType::Int:
				return static_cast<double>(object->AsInt());
			case ScriptType::Bool:
				return object->AsBool() ? 1.0 : 0.0;
			default:
				break;
		}
	}

	ScriptErr_SetString(ScriptErrorType::TypeError, "a float is required");
	return -1.0;
}

bool Script_IsTrue(ScriptObject* object)
{
	if (!object)
		return false;

	switch (object->Type())
	{
		case ScriptType::None:
			return false;
		case ScriptType::Bool:
			return object->AsBool();
		case ScriptType::Int:
			return object->AsInt() != 0;
		case ScriptType::Float:
			return object->AsFloat() != 0.0;
		case ScriptType::String:
			return !object->Bytes().empty();
		case ScriptType::Tuple:
		case ScriptType::List:
			return !object->Items().empty();
		case ScriptType::Dict:
			return !object->Entries().empty();
		default:
			return true;
	}
}

char* ScriptString_AsString(ScriptObject* object)
{
	if (!ScriptString_Check(object))
	{
		ScriptErr_SetString(ScriptErrorType::TypeError, "expected string");
		return nullptr;
	}

	return &object->MutableBytes()[0];
}

// ---------------------------------------------------------------------------
// Containers
// ---------------------------------------------------------------------------
static bool IsSequence(const ScriptObject* object)
{
	return object && (object->Type() == ScriptType::Tuple || object->Type() == ScriptType::List);
}

long ScriptTuple_Size(ScriptObject* object)
{
	if (!IsSequence(object))
	{
		ScriptErr_SetString(ScriptErrorType::SystemError, "bad argument to internal function");
		return -1;
	}

	return static_cast<long>(object->Items().size());
}

ScriptObject* ScriptTuple_GetItem(ScriptObject* object, long pos)
{
	if (!IsSequence(object))
	{
		ScriptErr_SetString(ScriptErrorType::SystemError, "bad argument to internal function");
		return nullptr;
	}

	if (pos < 0 || static_cast<std::size_t>(pos) >= object->Items().size())
	{
		ScriptErr_SetString(ScriptErrorType::IndexError, "tuple index out of range");
		return nullptr;
	}

	return object->Items()[static_cast<std::size_t>(pos)];
}

bool ScriptTuple_SetItem(ScriptObject* object, long pos, ScriptObject* value)
{
	if (!IsSequence(object) || pos < 0 || static_cast<std::size_t>(pos) >= object->Items().size())
	{
		if (value)
			Script_DecRef(value);
		ScriptErr_SetString(ScriptErrorType::IndexError, "tuple assignment index out of range");
		return false;
	}

	ScriptObject*& slot = ScriptObjectAccess::Items(object)[static_cast<std::size_t>(pos)];
	ScriptObject* old = slot;
	slot = value ? value : Script_None();
	Script_DecRef(old);
	return true;
}

bool ScriptList_Append(ScriptObject* list, ScriptObject* value)
{
	if (!ScriptList_Check(list) || !value)
	{
		ScriptErr_SetString(ScriptErrorType::SystemError, "bad argument to internal function");
		return false;
	}

	Script_IncRef(value);
	ScriptObjectAccess::Items(list).push_back(value);
	return true;
}

bool ScriptDict_SetItem(ScriptObject* dict, ScriptObject* key, ScriptObject* value)
{
	if (!ScriptDict_Check(dict) || !key || !value)
	{
		ScriptErr_SetString(ScriptErrorType::SystemError, "bad argument to internal function");
		return false;
	}

	Script_IncRef(value);
	for (auto& entry : ScriptObjectAccess::Entries(dict))
	{
		if (Script_Equals(entry.first, key))
		{
			Script_DecRef(entry.second);
			entry.second = value;
			return true;
		}
	}

	Script_IncRef(key);
	ScriptObjectAccess::Entries(dict).emplace_back(key, value);
	return true;
}

bool ScriptDict_SetItemString(ScriptObject* dict, const char* key, ScriptObject* value)
{
	ScriptObject* keyObject = ScriptString_New(key);
	const bool ok = ScriptDict_SetItem(dict, keyObject, value);
	Script_DecRef(keyObject);
	return ok;
}

ScriptObject* ScriptDict_GetItemString(ScriptObject* dict, const char* key)
{
	if (!ScriptDict_Check(dict) || !key)
		return nullptr;

	for (const auto& entry : dict->Entries())
	{
		if (ScriptString_Check(entry.first) && entry.first->Bytes() == key)
			return entry.second;
	}

	return nullptr;
}

bool ScriptDict_Next(ScriptObject* dict, long* pos, ScriptObject** key, ScriptObject** value)
{
	if (!ScriptDict_Check(dict) || !pos || *pos < 0 || static_cast<std::size_t>(*pos) >= dict->Entries().size())
		return false;

	const auto& entry = dict->Entries()[static_cast<std::size_t>(*pos)];
	if (key)
		*key = entry.first;
	if (value)
		*value = entry.second;
	++*pos;
	return true;
}

bool Script_Equals(const ScriptObject* a, const ScriptObject* b)
{
	if (a == b)
		return true;
	if (!a || !b)
		return false;

	const bool aNumber = ScriptNumber_Check(a);
	const bool bNumber = ScriptNumber_Check(b);
	if (aNumber && bNumber)
	{
		if (a->Type() == ScriptType::Float || b->Type() == ScriptType::Float)
		{
			auto toDouble = [](const ScriptObject* o)
			{
				return o->Type() == ScriptType::Float  ? o->AsFloat()
					   : o->Type() == ScriptType::Bool ? (o->AsBool() ? 1.0 : 0.0)
													   : static_cast<double>(o->AsInt());
			};
			return toDouble(a) == toDouble(b);
		}
		auto toInt = [](const ScriptObject* o)
		{
			return o->Type() == ScriptType::Bool ? (o->AsBool() ? 1 : 0) : o->AsInt();
		};
		return toInt(a) == toInt(b);
	}

	if (a->Type() != b->Type())
		return false;

	switch (a->Type())
	{
		case ScriptType::String:
			return a->Bytes() == b->Bytes();
		case ScriptType::Tuple:
		case ScriptType::List:
		{
			if (a->Items().size() != b->Items().size())
				return false;
			for (std::size_t i = 0; i < a->Items().size(); ++i)
			{
				if (!Script_Equals(a->Items()[i], b->Items()[i]))
					return false;
			}
			return true;
		}
		case ScriptType::Object:
			return a->Backend() == b->Backend() && a->Handle() == b->Handle();
		default:
			return false;
	}
}

// ---------------------------------------------------------------------------
// Errors
// ---------------------------------------------------------------------------
namespace
{
	struct PendingError
	{
			bool set = false;
			ScriptErrorType type = ScriptErrorType::RuntimeError;
			std::string message;
	};

	thread_local PendingError t_error;
}

void ScriptErr_SetString(ScriptErrorType type, const char* message)
{
	t_error.set = true;
	t_error.type = type;
	t_error.message = message ? message : "";
}

void ScriptErr_Format(ScriptErrorType type, const char* format, ...)
{
	char buffer[512 + 1];
	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	ScriptErr_SetString(type, buffer);
}

bool ScriptErr_Occurred()
{
	return t_error.set;
}

void ScriptErr_Clear()
{
	t_error.set = false;
	t_error.message.clear();
}

bool ScriptErr_Fetch(ScriptErrorType* type, std::string* message)
{
	if (!t_error.set)
		return false;

	if (type)
		*type = t_error.type;
	if (message)
		*message = std::move(t_error.message);
	ScriptErr_Clear();
	return true;
}

const char* ScriptErr_TypeName(ScriptErrorType type)
{
	switch (type)
	{
		case ScriptErrorType::RuntimeError:
			return "RuntimeError";
		case ScriptErrorType::TypeError:
			return "TypeError";
		case ScriptErrorType::ValueError:
			return "ValueError";
		case ScriptErrorType::IndexError:
			return "IndexError";
		case ScriptErrorType::KeyError:
			return "KeyError";
		case ScriptErrorType::OverflowError:
			return "OverflowError";
		case ScriptErrorType::AttributeError:
			return "AttributeError";
		case ScriptErrorType::SystemError:
			return "SystemError";
	}
	return "RuntimeError";
}
