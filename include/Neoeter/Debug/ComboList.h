#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Neoeter::Debug
{
	class ComboList
	{
		public:
			using LabelsProvider = std::function<std::vector<std::string>()>;
			using ValueGetter = std::function<int()>;
			using ValueSetter = std::function<void(int)>;

			void Add(const char* label, LabelsProvider labels, ValueGetter value, ValueSetter setter)
			{
				m_combos.push_back(Combo{label ? label : "", std::move(labels), std::move(value), std::move(setter)});
			}

			std::size_t Size() const
			{
				return m_combos.size();
			}

			const std::string& Label(std::size_t index) const
			{
				return m_combos[index].label;
			}

			std::vector<std::string> Labels(std::size_t index) const
			{
				const Combo& combo = m_combos[index];
				return combo.labels ? combo.labels() : std::vector<std::string>();
			}

			int Value(std::size_t index) const
			{
				const Combo& combo = m_combos[index];
				return combo.value ? combo.value() : 0;
			}

			void SetValue(std::size_t index, int value) const
			{
				const Combo& combo = m_combos[index];
				if (combo.setter)
					combo.setter(value);
			}

		private:
			struct Combo
			{
					std::string label;
					LabelsProvider labels;
					ValueGetter value;
					ValueSetter setter;
			};

			std::vector<Combo> m_combos;
	};
}
