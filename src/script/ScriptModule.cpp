#include "ScriptInternal.h"

#include <algorithm>
#include <cstring>
#include <memory>

namespace
{
	struct Registry
	{
			std::vector<std::unique_ptr<ScriptModuleData>> modules;
			std::vector<ScriptModuleData*> view;
			std::vector<ScriptModuleListener*> listeners;
	};

	Registry& GetRegistry()
	{
		static Registry s_registry;
		return s_registry;
	}

	void FreeModule(ScriptModuleData& module)
	{
		for (auto& constant : module.constants)
			Script_DecRef(constant.second);
		module.constants.clear();

		if (module.object)
			ScriptObjectAccess::Destroy(module.object);
		module.object = nullptr;
	}
}

const ScriptMethodDef* ScriptModuleData::FindMethod(const char* methodName) const
{
	for (const ScriptMethodDef& method : methods)
	{
		if (std::strcmp(method.name, methodName) == 0)
			return &method;
	}

	return nullptr;
}

ScriptObject* ScriptModuleData::FindConstant(const char* constantName) const
{
	for (const auto& constant : constants)
	{
		if (constant.first == constantName)
			return constant.second;
	}

	return nullptr;
}

ScriptModuleData* ScriptModule_Find(const char* name)
{
	for (ScriptModuleData* module : GetRegistry().view)
	{
		if (module->name == name)
			return module;
	}

	return nullptr;
}

const std::vector<ScriptModuleData*>& ScriptModule_All()
{
	return GetRegistry().view;
}

ScriptObject* ScriptModule_Create(const char* name, const ScriptMethodDef* methods)
{
	Registry& registry = GetRegistry();

	ScriptModuleData* module = ScriptModule_Find(name);
	const bool created = module == nullptr;
	if (created)
	{
		registry.modules.push_back(std::make_unique<ScriptModuleData>());
		module = registry.modules.back().get();
		module->name = name;
		module->object = ScriptObjectAccess::MakeModule(module);
		registry.view.push_back(module);
	}

	// Re-initialising a module adds (or replaces) functions, like Py_InitModule.
	const std::size_t firstNew = module->methods.size();
	for (const ScriptMethodDef* method = methods; method && method->name; ++method)
	{
		auto existing = std::find_if(module->methods.begin(), module->methods.end(),
									 [method](const ScriptMethodDef& m)
									 { return std::strcmp(m.name, method->name) == 0; });

		if (existing != module->methods.end())
			*existing = *method;
		else
			module->methods.push_back(*method);
	}

	for (ScriptModuleListener* listener : registry.listeners)
	{
		if (created)
			listener->OnModuleCreated(*module);
		else
			listener->OnModuleMethodsAdded(*module, firstNew);
	}

	return module->object;
}

int ScriptModule_AddObject(ScriptObject* moduleObject, const char* name, ScriptObject* value)
{
	if (!moduleObject || moduleObject->Type() != ScriptType::Module || !name || !value)
	{
		Script_XDECREF(value);
		ScriptErr_SetString(ScriptErrorType::TypeError, "ScriptModule_AddObject needs a module, a name and a value");
		return -1;
	}

	ScriptModuleData* module = moduleObject->Module();
	auto existing = std::find_if(module->constants.begin(), module->constants.end(),
								 [name](const std::pair<std::string, ScriptObject*>& c)
								 { return c.first == name; });

	if (existing != module->constants.end())
	{
		Script_DecRef(existing->second);
		existing->second = value;
	}
	else
		module->constants.emplace_back(name, value);

	for (ScriptModuleListener* listener : GetRegistry().listeners)
		listener->OnModuleConstant(*module, name, value);

	return 0;
}

int ScriptModule_AddIntConstant(ScriptObject* module, const char* name, long value)
{
	return ScriptModule_AddObject(module, name, ScriptInt_New(value));
}

int ScriptModule_AddStringConstant(ScriptObject* module, const char* name, const char* value)
{
	return ScriptModule_AddObject(module, name, ScriptString_New(value));
}

ScriptObject* ScriptModule_Invoke(const ScriptMethodDef* method, ScriptObject* args)
{
	if (!method || !method->function)
	{
		ScriptErr_SetString(ScriptErrorType::SystemError, "invalid native function");
		return nullptr;
	}

	ScriptObject* result = method->function(nullptr, args);
	if (!result && !ScriptErr_Occurred())
		ScriptErr_Format(ScriptErrorType::SystemError, "error return without exception set in %s", method->name);

	return result;
}

void ScriptModule_AddListener(ScriptModuleListener* listener)
{
	auto& listeners = GetRegistry().listeners;
	if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
		listeners.push_back(listener);
}

void ScriptModule_RemoveListener(ScriptModuleListener* listener)
{
	auto& listeners = GetRegistry().listeners;
	listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void ScriptModule_ResetRegistry()
{
	Registry& registry = GetRegistry();
	for (auto& module : registry.modules)
		FreeModule(*module);
	registry.view.clear();
	registry.modules.clear();
}
