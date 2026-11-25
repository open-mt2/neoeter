#include "ScriptInternal.h"

#include <cstring>

// A Py_BuildValue compatible builder. Varargs follow the C promotion rules,
// exactly like CPython: b/B/h/H/c/i are read as int, f/d as double.
namespace
{
	class Builder
	{
		public:
			Builder(const char* format, va_list* args) : m_cursor(format), m_args(args) {}

			ScriptObject* BuildTop()
			{
				const int count = CountItems(m_cursor, '\0');
				if (count < 0)
					return Fail("unmatched paren in format");

				if (count == 0)
					return Script_None();

				if (count == 1)
				{
					ScriptObject* item = BuildItem();
					return m_failed ? Discard(item) : item;
				}

				return BuildSequence(ScriptType::Tuple, '\0', count);
			}

		private:
			static bool IsSeparator(char c)
			{
				return c == ',' || c == ':' || c == ' ' || c == '\t';
			}

			// Counts the items until the matching close character.
			static int CountItems(const char* format, char endChar)
			{
				int level = 0;
				int count = 0;
				for (const char* p = format; *p; ++p)
				{
					const char c = *p;
					if (level == 0 && c == endChar)
						return count;

					switch (c)
					{
						case '(':
						case '[':
						case '{':
							if (level == 0)
								++count;
							++level;
							break;
						case ')':
						case ']':
						case '}':
							if (level == 0)
								return -1;
							--level;
							break;
						case '#':
						case '&':
						case ',':
						case ':':
						case ' ':
						case '\t':
							break;
						default:
							if (level == 0)
								++count;
					}
				}

				return endChar == '\0' && level == 0 ? count : -1;
			}

			ScriptObject* Fail(const char* message)
			{
				if (!m_failed)
					ScriptErr_SetString(ScriptErrorType::SystemError, message);
				m_failed = true;
				return nullptr;
			}

			static ScriptObject* Discard(ScriptObject* object)
			{
				Script_XDECREF(object);
				return nullptr;
			}

			void SkipSeparators()
			{
				while (IsSeparator(*m_cursor))
					++m_cursor;
			}

			ScriptObject* BuildSequence(ScriptType type, char endChar, int count)
			{
				ScriptObject* sequence = type == ScriptType::List ? ScriptList_New(count) : ScriptTuple_New(count);
				for (int i = 0; i < count; ++i)
				{
					ScriptObject* item = BuildItem();
					ScriptTuple_SetItem(sequence, i, item ? item : Script_None());
				}

				SkipSeparators();
				if (endChar != '\0')
				{
					if (*m_cursor != endChar)
						Fail("unmatched paren in format");
					else
						++m_cursor;
				}

				return m_failed ? Discard(sequence) : sequence;
			}

			ScriptObject* BuildDict(int count)
			{
				ScriptObject* dict = ScriptDict_New();
				for (int i = 0; i < count; i += 2)
				{
					ScriptObject* key = BuildItem();
					ScriptObject* value = i + 1 < count ? BuildItem() : nullptr;
					if (key && value)
						ScriptDict_SetItem(dict, key, value);
					Script_XDECREF(key);
					Script_XDECREF(value);
				}

				if (count % 2 != 0)
					Fail("Bad dict format");

				SkipSeparators();
				if (*m_cursor != '}')
					Fail("unmatched paren in format");
				else
					++m_cursor;

				return m_failed ? Discard(dict) : dict;
			}

			bool TakeLength(int* length)
			{
				if (*m_cursor != '#')
					return false;

				++m_cursor;
				*length = va_arg(*m_args, int);
				return true;
			}

			ScriptObject* BuildString(bool allowNull)
			{
				const char* value = va_arg(*m_args, const char*);
				int length = -1;
				const bool hasLength = TakeLength(&length);

				if (!value)
				{
					if (!allowNull && !hasLength)
						return Fail("NULL string passed to BuildValue");
					return Script_None();
				}

				if (!hasLength || length < 0)
					return ScriptString_New(value);

				return ScriptString_NewSize(value, static_cast<std::size_t>(length));
			}

			ScriptObject* BuildItem()
			{
				SkipSeparators();
				const char c = *m_cursor++;
				switch (c)
				{
					case '(':
						return BuildSequence(ScriptType::Tuple, ')', CountItems(m_cursor, ')'));
					case '[':
						return BuildSequence(ScriptType::List, ']', CountItems(m_cursor, ']'));
					case '{':
						return BuildDict(CountItems(m_cursor, '}'));
					case 'b':
					case 'B':
					case 'h':
					case 'i':
						return ScriptInt_New(va_arg(*m_args, int));
					case 'H':
						return ScriptInt_New(static_cast<unsigned short>(va_arg(*m_args, int)));
					case 'I':
						return ScriptInt_New(va_arg(*m_args, unsigned int));
					case 'l':
						return ScriptInt_New(va_arg(*m_args, long));
					case 'k':
						return ScriptInt_New(static_cast<std::int64_t>(va_arg(*m_args, unsigned long)));
					case 'L':
						return ScriptInt_New(va_arg(*m_args, long long));
					case 'K':
						return ScriptInt_New(static_cast<std::int64_t>(va_arg(*m_args, unsigned long long)));
					case 'n':
						return ScriptInt_New(static_cast<std::int64_t>(va_arg(*m_args, std::ptrdiff_t)));
					case 'c':
					{
						const char value = static_cast<char>(va_arg(*m_args, int));
						return ScriptString_NewSize(&value, 1);
					}
					case 'f':
					case 'd':
						return ScriptFloat_New(va_arg(*m_args, double));
					case 's':
					case 'y':
						return BuildString(false);
					case 'z':
						return BuildString(true);
					case 'O':
					case 'S':
					case 'N':
					{
						ScriptObject* value = va_arg(*m_args, ScriptObject*);
						if (!value)
							return ScriptErr_Occurred() ? (m_failed = true, nullptr) : Fail("NULL object passed to BuildValue");
						if (c != 'N')
							Script_IncRef(value);
						return value;
					}
					default:
						return Fail("bad format char passed to BuildValue");
				}
			}

			const char* m_cursor;
			va_list* m_args;
			bool m_failed = false;
	};
}

ScriptObject* Script_VaBuildValue(const char* format, va_list args)
{
	va_list copy;
	va_copy(copy, args);
	Builder builder(format ? format : "", &copy);
	ScriptObject* result = builder.BuildTop();
	va_end(copy);
	return result;
}

ScriptObject* Script_BuildValue(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	ScriptObject* result = Script_VaBuildValue(format, args);
	va_end(args);
	return result;
}
