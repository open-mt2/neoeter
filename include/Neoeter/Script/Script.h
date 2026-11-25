#pragma once

// ============================================================================
// Neoeter script core - a runtime-neutral binding layer.
// ============================================================================
// Native modules (app, net, wndMgr, ...) are written once against this API
// and exposed to every registered scripting backend (Python 2.7, .NET, ...).
//
// The API intentionally mirrors the historic CPython helper layer
// (PyTuple_GetInteger, Py_BuildValue, PyCallClassMemberFunc, ...) name for
// name and semantic for semantic, so the existing bindings were converted
// mechanically and behave exactly as before under Python:
//
//   PyObject                -> ScriptObject
//   Py_BuildValue           -> Script_BuildValue (same format strings)
//   Py_BuildNone/Exception  -> Script_BuildNone/Script_BuildException
//   PyTuple_Get*            -> ScriptTuple_Get*
//   PyCallClassMemberFunc   -> Script_CallMethod
//   Py_InitModule           -> ScriptModule_Create
//   PyModule_AddIntConstant -> ScriptModule_AddIntConstant
//
// Reference counting follows the CPython rules: functions returning
// ScriptObject* return a new reference unless documented as borrowed, and
// Script_CallMethod steals its argument tuple.
//
// Script objects are not thread safe; like the interpreter they replace, they
// must only be used from the script (main) thread.

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class ScriptBackend;
struct ScriptModuleData;

enum class ScriptType : std::uint8_t
{
	None,
	Bool,
	Int,
	Float,
	String,
	Tuple,
	List,
	Dict,
	Module,
	// An object owned by a scripting backend (a Python instance, a .NET object).
	Object,
};

enum class ScriptErrorType : std::uint8_t
{
	RuntimeError,
	TypeError,
	ValueError,
	IndexError,
	KeyError,
	OverflowError,
	AttributeError,
	SystemError,
};

class ScriptObject
{
	public:
		ScriptObject(const ScriptObject&) = delete;
		ScriptObject& operator=(const ScriptObject&) = delete;

		ScriptType Type() const { return m_type; }

		bool AsBool() const { return m_value.b; }
		std::int64_t AsInt() const { return m_value.i; }
		double AsFloat() const { return m_value.f; }

		// String payload. Strings are raw byte sequences (no encoding is
		// implied); the buffer is always NUL terminated.
		const std::string& Bytes() const { return m_bytes; }
		std::string& MutableBytes() { return m_bytes; }

		// Tuple / List items (owned references).
		const std::vector<ScriptObject*>& Items() const { return m_items; }

		// Dict entries in insertion order (owned references).
		const std::vector<std::pair<ScriptObject*, ScriptObject*>>& Entries() const { return m_entries; }

		ScriptModuleData* Module() const { return m_module; }

		// Object payload: the owning backend and its opaque handle.
		ScriptBackend* Backend() const { return m_backend; }
		void* Handle() const { return m_handle; }
		void SetHandle(void* handle) { m_handle = handle; }

		// Scalar/container reference count (not used for Object, whose lifetime
		// belongs to its backend).
		std::int32_t RefCount() const { return m_refcount; }

	private:
		explicit ScriptObject(ScriptType type) : m_type(type) {}
		~ScriptObject() = default;

		friend struct ScriptObjectAccess;

		std::int32_t m_refcount = 1;
		ScriptType m_type;
		union Value
		{
				bool b;
				std::int64_t i;
				double f;
		} m_value{};
		std::string m_bytes;
		std::vector<ScriptObject*> m_items;
		std::vector<std::pair<ScriptObject*, ScriptObject*>> m_entries;
		ScriptModuleData* m_module = nullptr;
		ScriptBackend* m_backend = nullptr;
		void* m_handle = nullptr;
};

// ---------------------------------------------------------------------------
// Reference counting
// ---------------------------------------------------------------------------
void Script_IncRef(ScriptObject* object);
void Script_DecRef(ScriptObject* object);

#define Script_INCREF(o) Script_IncRef(o)
#define Script_DECREF(o) Script_DecRef(o)
#define Script_XINCREF(o)        \
	do {                         \
		ScriptObject* _so = (o); \
		if (_so)                 \
			Script_IncRef(_so);  \
	} while (0)
#define Script_XDECREF(o)        \
	do {                         \
		ScriptObject* _so = (o); \
		if (_so)                 \
			Script_DecRef(_so);  \
	} while (0)

// ---------------------------------------------------------------------------
// Construction (new references)
// ---------------------------------------------------------------------------
ScriptObject* Script_None(); // borrowed, immortal
ScriptObject* ScriptBool_New(bool value);
ScriptObject* ScriptInt_New(std::int64_t value);
ScriptObject* ScriptFloat_New(double value);
ScriptObject* ScriptString_New(const char* value);
ScriptObject* ScriptString_NewSize(const char* data, std::size_t size);
ScriptObject* ScriptTuple_New(std::size_t size); // items start as None
ScriptObject* ScriptList_New(std::size_t size);	 // items start as None
ScriptObject* ScriptDict_New();

// Creates the wrapper for a backend object. The backend owns the wrapper and
// releases it with ScriptObject_FreeForeign.
ScriptObject* ScriptObject_NewForeign(ScriptBackend* backend, void* handle);
void ScriptObject_FreeForeign(ScriptObject* object);

// ---------------------------------------------------------------------------
// Type checks and conversions
// ---------------------------------------------------------------------------
inline bool ScriptNone_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::None; }
inline bool ScriptBool_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::Bool; }
inline bool ScriptInt_Check(const ScriptObject* o) { return o && (o->Type() == ScriptType::Int || o->Type() == ScriptType::Bool); }
inline bool ScriptFloat_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::Float; }
inline bool ScriptNumber_Check(const ScriptObject* o) { return ScriptInt_Check(o) || ScriptFloat_Check(o); }
inline bool ScriptString_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::String; }
inline bool ScriptTuple_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::Tuple; }
inline bool ScriptList_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::List; }
inline bool ScriptDict_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::Dict; }
inline bool ScriptForeign_Check(const ScriptObject* o) { return o && o->Type() == ScriptType::Object; }

// Python semantics: numbers convert (floats truncate); anything else sets a
// TypeError and returns -1.
std::int64_t Script_AsLongLong(ScriptObject* object);
std::uint64_t Script_AsUnsignedLongLong(ScriptObject* object);
double Script_AsDouble(ScriptObject* object);
bool Script_IsTrue(ScriptObject* object);
// Borrowed pointer into the string; nullptr (and TypeError) for non strings.
char* ScriptString_AsString(ScriptObject* object);

// ---------------------------------------------------------------------------
// Containers
// ---------------------------------------------------------------------------
// Size of a tuple or list, -1 (SystemError) otherwise.
long ScriptTuple_Size(ScriptObject* object);
// Borrowed item of a tuple or list; nullptr (IndexError) when out of range.
ScriptObject* ScriptTuple_GetItem(ScriptObject* object, long pos);
// Steals value.
bool ScriptTuple_SetItem(ScriptObject* object, long pos, ScriptObject* value);
// Does not steal value.
bool ScriptList_Append(ScriptObject* list, ScriptObject* value);
// Does not steal key/value.
bool ScriptDict_SetItem(ScriptObject* dict, ScriptObject* key, ScriptObject* value);
bool ScriptDict_SetItemString(ScriptObject* dict, const char* key, ScriptObject* value);
// Borrowed; nullptr when missing (no error set).
ScriptObject* ScriptDict_GetItemString(ScriptObject* dict, const char* key);
// Iterates dict entries. *pos starts at 0; key/value are borrowed.
bool ScriptDict_Next(ScriptObject* dict, long* pos, ScriptObject** key, ScriptObject** value);

bool Script_Equals(const ScriptObject* a, const ScriptObject* b);

// ---------------------------------------------------------------------------
// Errors (one pending error per thread, like PyErr_*)
// ---------------------------------------------------------------------------
void ScriptErr_SetString(ScriptErrorType type, const char* message);
void ScriptErr_Format(ScriptErrorType type, const char* format, ...);
bool ScriptErr_Occurred();
void ScriptErr_Clear();
// Moves the pending error out. Returns false when there is none.
bool ScriptErr_Fetch(ScriptErrorType* type, std::string* message);
const char* ScriptErr_TypeName(ScriptErrorType type);

// ---------------------------------------------------------------------------
// Historic helper layer (Py* -> Script* one to one)
// ---------------------------------------------------------------------------
// Sets RuntimeError(format) (or clears the error for nullptr) and returns
// None, exactly like the old Py_BuildException.
ScriptObject* Script_BuildException(const char* format = nullptr, ...);
// Sets TypeError and returns nullptr.
ScriptObject* Script_BadArgument();
ScriptObject* Script_BuildNone();
ScriptObject* Script_BuildEmptyTuple();
// Py_BuildValue compatible: i b h B H I l k L K n c f d s z s# z# y y# O S N
// and () [] {} groups; "" builds None, several top level items build a tuple.
ScriptObject* Script_BuildValue(const char* format, ...);
ScriptObject* Script_VaBuildValue(const char* format, va_list args);

// Native pointers cross the script boundary as pointer wide integers.
ScriptObject* Script_BuildHandle(const void* handle);

bool ScriptTuple_GetString(ScriptObject* args, int pos, char** ret);
bool ScriptTuple_GetInteger(ScriptObject* args, int pos, unsigned char* ret);
bool ScriptTuple_GetInteger(ScriptObject* args, int pos, int* ret);
bool ScriptTuple_GetInteger(ScriptObject* args, int pos, unsigned short* ret);
bool ScriptTuple_GetByte(ScriptObject* args, int pos, unsigned char* ret);
bool ScriptTuple_GetUnsignedInteger(ScriptObject* args, int pos, unsigned int* ret);
bool ScriptTuple_GetLong(ScriptObject* args, int pos, long* ret);
bool ScriptTuple_GetUnsignedLong(ScriptObject* args, int pos, unsigned long* ret);
bool ScriptTuple_GetFloat(ScriptObject* args, int pos, float* ret);
bool ScriptTuple_GetDouble(ScriptObject* args, int pos, double* ret);
bool ScriptTuple_GetObject(ScriptObject* args, int pos, ScriptObject** ret);
bool ScriptTuple_GetBoolean(ScriptObject* args, int pos, bool* ret);
bool ScriptTuple_GetHandle(ScriptObject* args, int pos, void** handle);

template <typename T>
bool ScriptTuple_GetHandle(ScriptObject* args, int pos, T** handle)
{
	void* raw;
	if (!ScriptTuple_GetHandle(args, pos, &raw))
		return false;

	*handle = static_cast<T*>(raw);
	return true;
}

// Calls object.name(*args). args is stolen (may be nullptr). Returns false
// when object is null, has no such callable attribute or the call raised (the
// backend reports the exception).
bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args);
// As above; *result is truthiness of a numeric result, true otherwise.
bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args, bool* result);
// As above; false when the result is not a number.
bool Script_CallMethod(ScriptObject* object, const char* name, ScriptObject* args, long* result);
// Returns a new reference to the result or nullptr.
ScriptObject* Script_CallMethodObject(ScriptObject* object, const char* name, ScriptObject* args);
bool Script_HasAttr(ScriptObject* object, const char* name);

// ---------------------------------------------------------------------------
// Native modules
// ---------------------------------------------------------------------------
typedef ScriptObject* (*ScriptCFunction)(ScriptObject* self, ScriptObject* args);

enum
{
	SCRIPT_VARARGS = 0x0001,
};

struct ScriptMethodDef
{
		const char* name;
		ScriptCFunction function;
		int flags;
		const char* doc;
};

struct ScriptModuleData
{
		std::string name;
		std::vector<ScriptMethodDef> methods;
		// Constants in registration order (owned references).
		std::vector<std::pair<std::string, ScriptObject*>> constants;
		ScriptObject* object = nullptr;

		const ScriptMethodDef* FindMethod(const char* methodName) const;
		ScriptObject* FindConstant(const char* constantName) const;
};

// Registers (or extends) the module and returns it (borrowed). The method
// table is terminated by an entry with a null name and is copied.
ScriptObject* ScriptModule_Create(const char* name, const ScriptMethodDef* methods);
int ScriptModule_AddObject(ScriptObject* module, const char* name, ScriptObject* value); // steals
int ScriptModule_AddIntConstant(ScriptObject* module, const char* name, long value);
int ScriptModule_AddStringConstant(ScriptObject* module, const char* name, const char* value);

ScriptModuleData* ScriptModule_Find(const char* name);
const std::vector<ScriptModuleData*>& ScriptModule_All();

// Invokes a module function from a backend. args is borrowed. Returns a new
// reference or nullptr; a pending error may accompany a non-null result (the
// historic Script_BuildException contract) and must be reported by the
// backend.
ScriptObject* ScriptModule_Invoke(const ScriptMethodDef* method, ScriptObject* args);

// Backends observe module registration so modules created after the runtime
// starts (and constants added afterwards) are exposed too.
class ScriptModuleListener
{
	public:
		virtual ~ScriptModuleListener() = default;
		virtual void OnModuleCreated(ScriptModuleData& module) = 0;
		virtual void OnModuleMethodsAdded(ScriptModuleData& module, std::size_t firstNewMethod) = 0;
		virtual void OnModuleConstant(ScriptModuleData& module, const std::string& name, ScriptObject* value) = 0;
};

void ScriptModule_AddListener(ScriptModuleListener* listener);
void ScriptModule_RemoveListener(ScriptModuleListener* listener);

// Drops every module (tests and shutdown).
void ScriptModule_ResetRegistry();
