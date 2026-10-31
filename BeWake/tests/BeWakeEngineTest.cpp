#include "../src/BeWakeEngine.h"

#include <cassert>
#include <cmath>
#include <iostream>


namespace {

bool
nearly_equal(double first, double second)
{
	return std::abs(first - second) < 0.000001;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_hue_normalisation()
{
	assert(nearly_equal(bewake::NormaliseHue(0.0), 0.0));
	assert(nearly_equal(bewake::NormaliseHue(1.0), 0.0));
	assert(nearly_equal(bewake::NormaliseHue(-0.25), 0.75));
	assert(nearly_equal(bewake::NormaliseHue(2.5), 0.5));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_hsl_primary_colours()
{
	const bewake::Colour red = bewake::HslToRgb(0.0, 1.0, 0.5);
	const bewake::Colour green = bewake::HslToRgb(1.0 / 3.0, 1.0, 0.5);
	const bewake::Colour blue = bewake::HslToRgb(2.0 / 3.0, 1.0, 0.5);
	assert(red.red > 250 && red.green < 5 && red.blue < 5);
	assert(green.red < 5 && green.green > 250 && green.blue < 5);
	assert(blue.red < 5 && blue.green < 5 && blue.blue > 250);
	assert(bewake::HslToRgb(0.7, 0.0, 0.5)
		== (bewake::Colour { 127, 127, 127, 255 }));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_lines_move_and_fade_within_bounds()
{
	bewake::Engine engine(42);
	engine.Resize(320, 200);
	assert(engine.Lines().size() == 50);
	const bewake::WakeLine original = engine.Lines().front();
	engine.Step();
	assert(engine.Lines().front().x1 != original.x1
		|| engine.Lines().front().y1 != original.y1);
	assert(nearly_equal(engine.Lines()[1].lightness, 0.49));

	for (int32_t step = 0; step < 5000; step++) {
		engine.Step();
		for (const bewake::WakeLine& line : engine.Lines()) {
			assert(line.x1 >= 0.0 && line.x1 <= engine.Width() - 1.0);
			assert(line.y1 >= 0.0 && line.y1 <= engine.Height() - 1.0);
			assert(line.x2 >= 0.0 && line.x2 <= engine.Width() - 1.0);
			assert(line.y2 >= 0.0 && line.y2 <= engine.Height() - 1.0);
			assert(line.lightness >= 0.0 && line.lightness <= 0.5);
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_colour_modes_apply_immediately()
{
	bewake::Engine engine(7);
	engine.Resize(100, 80);
	engine.SetFixedHue(0.25);
	engine.SetColourMode(bewake::ColourMode::Fixed);
	assert(nearly_equal(engine.Lines().front().hue, 0.25));
	engine.Step();
	assert(nearly_equal(engine.Lines().front().hue, 0.25));

	engine.SetColourMode(bewake::ColourMode::Cycle);
	const double initialHue = engine.Lines().front().hue;
	engine.Step();
	assert(!nearly_equal(engine.Lines().front().hue, initialHue));
	engine.SetColourMode(bewake::ColourMode::Random);
	assert(engine.CurrentColourMode() == bewake::ColourMode::Random);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_seeded_engines_are_deterministic()
{
	bewake::Engine first(1234);
	bewake::Engine second(1234);
	first.Resize(64, 48);
	second.Resize(64, 48);
	first.SetColourMode(bewake::ColourMode::Random);
	second.SetColourMode(bewake::ColourMode::Random);
	for (int32_t step = 0; step < 1500; step++) {
		first.Step();
		second.Step();
	}
	const std::vector<bewake::WakeLine>& firstLines = first.Lines();
	const std::vector<bewake::WakeLine>& secondLines = second.Lines();
	assert(firstLines.size() == secondLines.size());
	for (std::size_t index = 0; index < firstLines.size(); index++) {
		assert(nearly_equal(firstLines[index].x1, secondLines[index].x1));
		assert(nearly_equal(firstLines[index].y1, secondLines[index].y1));
		assert(nearly_equal(firstLines[index].x2, secondLines[index].x2));
		assert(nearly_equal(firstLines[index].y2, secondLines[index].y2));
		assert(nearly_equal(firstLines[index].hue, secondLines[index].hue));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_tiny_canvas_is_safe()
{
	bewake::Engine engine(1);
	engine.Resize(0, 0);
	for (int32_t step = 0; step < 100; step++)
		engine.Step();
	assert(engine.Width() == 1);
	assert(engine.Height() == 1);
	for (const bewake::WakeLine& line : engine.Lines()) {
		assert(line.x1 == 0.0 && line.y1 == 0.0);
		assert(line.x2 == 0.0 && line.y2 == 0.0);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_hue_normalisation();
	test_hsl_primary_colours();
	test_lines_move_and_fade_within_bounds();
	test_colour_modes_apply_immediately();
	test_seeded_engines_are_deterministic();
	test_tiny_canvas_is_safe();
	std::cout << "BeWake engine tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
