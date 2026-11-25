#include <Neoeter/Script/Script.h>
#include <Neoeter/Script/ScriptBackend.h>

#include <support/TestAssert.h>

#include <cstring>
#include <string>

namespace
{
	ScriptObject* Echo(ScriptObject*, ScriptObject* args)
	{
		Script_IncRef(args);
		return args;
	}

	ScriptObject* Fails(ScriptObject*, ScriptObject*)
	{
		return Script_BuildException("broken %d", 7);
	}

	struct RecordingListener : ScriptModuleListener
	{
			int created = 0;
			int added = 0;
			std::string lastConstant;

			void OnModuleCreated(ScriptModuleData&) override { ++created; }
			void OnModuleMethodsAdded(ScriptModuleData&, std::size_t) override { ++added; }
			void OnModuleConstant(ScriptModuleData&, const std::string& name, ScriptObject*) override { lastConstant = name; }
	};

	// Minimal backend: objects are ints; method "Twice" doubles its argument.
	struct FakeBackend : ScriptBackend
	{
			int strongRefs = 0;

			const char* Name() const override { return "fake"; }
			void IncRef(ScriptObject*) override { ++strongRefs; }
			void DecRef(ScriptObject*) override { --strongRefs; }
			bool HasAttr(ScriptObject*, const char* name) override { return std::strcmp(name, "Twice") == 0; }
			std::string Repr(ScriptObject*) override { return "<fake>"; }

			ScriptCallStatus CallMethod(ScriptObject*, const char* name, ScriptObject* args, ScriptObject** result) override
			{
				if (std::strcmp(name, "Twice") != 0)
					return ScriptCallStatus::NoMethod;

				int value = 0;
				if (!ScriptTuple_GetInteger(args, 0, &value))
					return ScriptCallStatus::Error;

				*result = ScriptInt_New(value * 2);
				return ScriptCallStatus::Ok;
			}
	};
}

static void TestBuildValue()
{
	TEST_SECTION("Script_BuildValue");

	ScriptObject* none = Script_BuildValue("");
	TEST_ASSERT(ScriptNone_Check(none));

	ScriptObject* single = Script_BuildValue("i", 42);
	TEST_ASSERT(ScriptInt_Check(single) && single->AsInt() == 42);
	Script_DecRef(single);

	ScriptObject* empty = Script_BuildValue("()");
	TEST_ASSERT(ScriptTuple_Check(empty) && ScriptTuple_Size(empty) == 0);
	Script_DecRef(empty);

	// 'b' builds an int like CPython, not a bool.
	ScriptObject* flag = Script_BuildValue("b", true);
	TEST_ASSERT(flag->Type() == ScriptType::Int && flag->AsInt() == 1);
	Script_DecRef(flag);

	const char raw[] = {'a', '\0', 'b'};
	ScriptObject* nested = Script_BuildValue("(is#[fd]){si}", 1, raw, 3, 1.5f, 2.5, "k", 9);
	TEST_ASSERT(ScriptTuple_Check(nested) && ScriptTuple_Size(nested) == 2);
	ScriptObject* inner = ScriptTuple_GetItem(nested, 0);
	TEST_ASSERT(ScriptTuple_Size(inner) == 3);
	TEST_ASSERT(ScriptTuple_GetItem(inner, 1)->Bytes() == std::string(raw, 3));
	ScriptObject* list = ScriptTuple_GetItem(inner, 2);
	TEST_ASSERT(ScriptList_Check(list) && Script_AsDouble(ScriptTuple_GetItem(list, 0)) == 1.5);
	ScriptObject* dict = ScriptTuple_GetItem(nested, 1);
	TEST_ASSERT(Script_AsLongLong(ScriptDict_GetItemString(dict, "k")) == 9);
	Script_DecRef(nested);

	ScriptObject* stolen = ScriptString_New("owned");
	ScriptObject* withN = Script_BuildValue("sNsi", "a", stolen, "b", 3);
	TEST_ASSERT(ScriptTuple_Size(withN) == 4 && stolen->RefCount() == 1);
	Script_DecRef(withN);

	ScriptObject* nullString = Script_BuildValue("z", static_cast<const char*>(nullptr));
	TEST_ASSERT(ScriptNone_Check(nullString));

	TEST_ASSERT(Script_BuildValue("x") == nullptr && ScriptErr_Occurred());
	ScriptErr_Clear();
	TEST_PASS("formats, groups, s#, N stealing and errors");
}

static void TestTupleGetters()
{
	TEST_SECTION("ScriptTuple_Get*");

	ScriptObject* args = Script_BuildValue("(ifsLi)", -3, 2.75, "text", 1LL << 40, 0);

	int i = 0;
	TEST_ASSERT(ScriptTuple_GetInteger(args, 0, &i) && i == -3);
	float f = 0.0f;
	TEST_ASSERT(ScriptTuple_GetFloat(args, 1, &f) && f == 2.75f);
	// Floats truncate when read as integers.
	TEST_ASSERT(ScriptTuple_GetInteger(args, 1, &i) && i == 2);
	char* s = nullptr;
	TEST_ASSERT(ScriptTuple_GetString(args, 2, &s) && std::strcmp(s, "text") == 0);
	TEST_ASSERT(!ScriptTuple_GetString(args, 0, &s));
	void* handle = nullptr;
	TEST_ASSERT(ScriptTuple_GetHandle(args, 3, &handle) && reinterpret_cast<std::intptr_t>(handle) == (1LL << 40));
	bool b = true;
	TEST_ASSERT(ScriptTuple_GetBoolean(args, 4, &b) && !b);
	TEST_ASSERT(!ScriptTuple_GetInteger(args, 5, &i));

	// Like the historic helper: a non number still "succeeds" but leaves an error.
	TEST_ASSERT(ScriptTuple_GetInteger(args, 2, &i) && i == -1 && ScriptErr_Occurred());
	ScriptErr_Clear();

	Script_DecRef(args);
	TEST_PASS("integer, float, string, handle, boolean and range checks");
}

static void TestErrors()
{
	TEST_SECTION("errors");

	ScriptObject* result = Script_BuildException("bad value %d", 5);
	TEST_ASSERT(ScriptNone_Check(result));
	ScriptErrorType type;
	std::string message;
	TEST_ASSERT(ScriptErr_Fetch(&type, &message));
	TEST_ASSERT(type == ScriptErrorType::RuntimeError && message == "bad value 5");
	TEST_ASSERT(!ScriptErr_Occurred());

	TEST_ASSERT(Script_BadArgument() == nullptr);
	TEST_ASSERT(ScriptErr_Fetch(&type, nullptr) && type == ScriptErrorType::TypeError);
	TEST_PASS("BuildException returns None with RuntimeError, BadArgument returns null");
}

static void TestModules()
{
	TEST_SECTION("module registry");

	RecordingListener listener;
	ScriptModule_AddListener(&listener);

	static const ScriptMethodDef s_methods[] = {
		{"Echo", Echo, SCRIPT_VARARGS, nullptr},
		{nullptr, nullptr, 0, nullptr},
	};
	ScriptObject* module = ScriptModule_Create("test", s_methods);
	TEST_ASSERT(module && listener.created == 1);
	ScriptModule_AddIntConstant(module, "ANSWER", 42);
	TEST_ASSERT(listener.lastConstant == "ANSWER");

	static const ScriptMethodDef s_more[] = {
		{"Fails", Fails, SCRIPT_VARARGS, nullptr},
		{nullptr, nullptr, 0, nullptr},
	};
	TEST_ASSERT(ScriptModule_Create("test", s_more) == module && listener.added == 1);

	ScriptModuleData* data = ScriptModule_Find("test");
	TEST_ASSERT(data && data->methods.size() == 2);
	TEST_ASSERT(Script_AsLongLong(data->FindConstant("ANSWER")) == 42);

	ScriptObject* args = Script_BuildValue("(i)", 5);
	ScriptObject* echoed = ScriptModule_Invoke(data->FindMethod("Echo"), args);
	TEST_ASSERT(echoed == args);
	Script_DecRef(echoed);
	Script_DecRef(args);

	ScriptObject* failed = ScriptModule_Invoke(data->FindMethod("Fails"), nullptr);
	TEST_ASSERT(ScriptNone_Check(failed) && ScriptErr_Occurred());
	ScriptErr_Clear();

	ScriptModule_RemoveListener(&listener);
	ScriptModule_ResetRegistry();
	TEST_ASSERT(ScriptModule_Find("test") == nullptr);
	TEST_PASS("create, extend, constants, invoke and listeners");
}

static void TestCallMethod()
{
	TEST_SECTION("Script_CallMethod");

	FakeBackend backend;
	ScriptObject* object = ScriptObject_NewForeign(&backend, &backend);

	long value = 0;
	TEST_ASSERT(Script_CallMethod(object, "Twice", Script_BuildValue("(i)", 21), &value) && value == 42);
	TEST_ASSERT(!Script_CallMethod(object, "Missing", Script_BuildValue("()")));
	TEST_ASSERT(!Script_CallMethod(nullptr, "Twice", Script_BuildValue("(i)", 1)));
	TEST_ASSERT(Script_HasAttr(object, "Twice") && !Script_HasAttr(object, "Missing"));

	Script_INCREF(object);
	TEST_ASSERT(backend.strongRefs == 1);
	Script_DECREF(object);
	TEST_ASSERT(backend.strongRefs == 0);

	ScriptObject_FreeForeign(object);
	TEST_PASS("dispatch, missing methods, null receivers and foreign refcounts");
}

static void TestActiveBackend()
{
	TEST_SECTION("Active backend");

	struct RunnerBackend : FakeBackend
	{
			std::string ranFile;
			int updates = 0;

			bool RunFile(const char* path) override
			{
				ranFile = path;
				return true;
			}
			void Update() override { ++updates; }
	};

	TEST_ASSERT(!Script_GetActiveBackend() && !Script_RunFile("system.py"));
	Script_UpdateActiveBackend();

	FakeBackend plain;
	Script_SetActiveBackend(&plain);
	TEST_ASSERT(!Script_RunFile("system.py")); // RunFile is optional

	RunnerBackend runner;
	Script_SetActiveBackend(&runner);
	TEST_ASSERT(Script_GetActiveBackend() == &runner);
	TEST_ASSERT(Script_RunFile("intro.py") && runner.ranFile == "intro.py");
	TEST_ASSERT(!Script_RunFile(nullptr));
	Script_UpdateActiveBackend();
	Script_UpdateActiveBackend();
	TEST_ASSERT(runner.updates == 2);

	Script_SetActiveBackend(nullptr);
	TEST_PASS("RunFile and Update reach only the active backend");
}

int main()
{
	TestBuildValue();
	TestTupleGetters();
	TestErrors();
	TestModules();
	TestCallMethod();
	TestActiveBackend();
	return 0;
}
