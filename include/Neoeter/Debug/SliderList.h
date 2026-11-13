#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Neoeter::Debug
{
	class SliderList
	{
		public:
			using ValueGetter = std::function<float()>;
			using ValueSetter = std::function<void(float)>;

			void Add(const char* label, float minimum, float maximum, ValueGetter value, ValueSetter setter)
			{
				m_sliders.push_back(Slider{label ? label : "", minimum, maximum, std::move(value), std::move(setter)});
			}

			std::size_t Size() const
			{
				return m_sliders.size();
			}

			const std::string& Label(std::size_t index) const
			{
				return m_sliders[index].label;
			}

			float Minimum(std::size_t index) const
			{
				return m_sliders[index].minimum;
			}

			float Maximum(std::size_t index) const
			{
				return m_sliders[index].maximum;
			}

			float Value(std::size_t index) const
			{
				const Slider& slider = m_sliders[index];
				return slider.value ? slider.value() : 0.0f;
			}

			void SetValue(std::size_t index, float value) const
			{
				const Slider& slider = m_sliders[index];
				if (slider.setter)
					slider.setter(value);
			}

		private:
			struct Slider
			{
					std::string label;
					float minimum;
					float maximum;
					ValueGetter value;
					ValueSetter setter;
			};

			std::vector<Slider> m_sliders;
	};
}
