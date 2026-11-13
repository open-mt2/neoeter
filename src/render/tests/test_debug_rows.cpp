#include <support/TestAssert.h>

#include <Neoeter/Debug/Debug.h>
#include <Neoeter/Debug/RowList.h>
#include <Neoeter/Debug/SliderList.h>

#include <string>

int main()
{
	TEST_SECTION("debug rows");

	Neoeter::Debug::RowList rows;
	TEST_ASSERT(rows.Size() == 0);

	rows.Add("First", []
			 { return std::string("one"); });
	rows.Add("Second", []
			 { return std::string("two"); });
	rows.Add("Third", []
			 { return std::string("three"); });

	TEST_ASSERT(rows.Size() == 3);
	TEST_ASSERT(rows.Label(0) == "First");
	TEST_ASSERT(rows.Label(1) == "Second");
	TEST_ASSERT(rows.Label(2) == "Third");
	TEST_ASSERT(rows.Value(0) == "one");
	TEST_ASSERT(rows.Value(1) == "two");
	TEST_ASSERT(rows.Value(2) == "three");

	int counter = 0;
	rows.Add("Counter", [&counter]
			 { return std::to_string(++counter); });

	TEST_ASSERT(rows.Value(3) == "1");
	TEST_ASSERT(rows.Value(3) == "2");
	TEST_ASSERT(rows.Value(3) == "3");

	Neoeter::Debug::RowList empty;
	empty.Add("Null", Neoeter::Debug::RowList::ValueProvider());
	TEST_ASSERT(empty.Size() == 1);
	TEST_ASSERT(empty.Value(0).empty());

	Neoeter::Debug::RowList anonymous;
	anonymous.Add(nullptr, []
				  { return std::string("value"); });
	TEST_ASSERT(anonymous.Label(0).empty());
	TEST_ASSERT(anonymous.Value(0) == "value");

	Neoeter::Debug::SliderList sliders;
	TEST_ASSERT(sliders.Size() == 0);

	float hour = 12.0f;
	float applied = 0.0f;
	sliders.Add(
		"Hour",
		0.0f,
		24.0f,
		[&hour]
		{ return hour; },
		[&applied](float value)
		{ applied = value; });

	TEST_ASSERT(sliders.Size() == 1);
	TEST_ASSERT(sliders.Label(0) == "Hour");
	TEST_ASSERT(sliders.Minimum(0) == 0.0f);
	TEST_ASSERT(sliders.Maximum(0) == 24.0f);
	TEST_ASSERT(sliders.Value(0) == 12.0f);

	hour = 6.5f;
	TEST_ASSERT(sliders.Value(0) == 6.5f);

	sliders.SetValue(0, 18.5f);
	TEST_ASSERT(applied == 18.5f);

	Neoeter::Debug::SliderList emptySliders;
	emptySliders.Add(
		nullptr,
		-1.0f,
		1.0f,
		Neoeter::Debug::SliderList::ValueGetter(),
		Neoeter::Debug::SliderList::ValueSetter());
	TEST_ASSERT(emptySliders.Size() == 1);
	TEST_ASSERT(emptySliders.Label(0).empty());
	TEST_ASSERT(emptySliders.Value(0) == 0.0f);
	emptySliders.SetValue(0, 1.0f);

#if defined(NEOETER_DEBUG_UI)
	int calls = 0;
	Neoeter::Debug::AddRow("Registered", []
						   { return std::string("value"); });
	Neoeter::Debug::AddRow("Counted", [&calls]
						   { return std::to_string(++calls); });
	Neoeter::Debug::AddSlider(
		"Registered",
		0.0f,
		1.0f,
		[]
		{ return 0.5f; },
		[](float) {});
#endif

	return 0;
}
