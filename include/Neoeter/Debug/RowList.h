#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Neoeter::Debug
{
	class RowList
	{
		public:
			using ValueProvider = std::function<std::string()>;

			void Add(const char* label, ValueProvider value)
			{
				m_rows.push_back(Row{label ? label : "", std::move(value)});
			}

			std::size_t Size() const
			{
				return m_rows.size();
			}

			const std::string& Label(std::size_t index) const
			{
				return m_rows[index].label;
			}

			std::string Value(std::size_t index) const
			{
				const Row& row = m_rows[index];
				return row.value ? row.value() : std::string();
			}

		private:
			struct Row
			{
				std::string label;
				ValueProvider value;
			};

			std::vector<Row> m_rows;
	};
}
