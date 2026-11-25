#pragma once

#include <Neoeter/Script/Script.h>

// Private construction/mutation access for the script core implementation.
struct ScriptObjectAccess
{
		static ScriptObject* Make(ScriptType type);
		static ScriptObject* MakeModule(ScriptModuleData* module);
		static void Destroy(ScriptObject* object);

		static ScriptObject::Value& Value(ScriptObject* object) { return object->m_value; }
		static ScriptBackend*& Backend(ScriptObject* object) { return object->m_backend; }
		static ScriptModuleData*& Module(ScriptObject* object) { return object->m_module; }
		static std::int32_t& RefCount(ScriptObject* object) { return object->m_refcount; }
		static std::vector<ScriptObject*>& Items(ScriptObject* object) { return object->m_items; }
		static std::vector<std::pair<ScriptObject*, ScriptObject*>>& Entries(ScriptObject* object) { return object->m_entries; }
};
