#include "../src/BrainWashEngine.h"

#include <algorithm>
#include <cassert>
#include <iostream>


namespace {

void
test_setting_conversions_are_bounded()
{
	assert(brainwash::DrawingLinesPerFrame(-10)
		== brainwash::DrawingLinesPerFrame(1));
	assert(brainwash::DrawingLinesPerFrame(9)
		> brainwash::DrawingLinesPerFrame(5));
	assert(brainwash::DrawingLinesPerFrame(5)
		> brainwash::DrawingLinesPerFrame(1));
	assert(brainwash::DrawingGapMicroseconds(-1) == 0);
	assert(brainwash::DrawingGapMicroseconds(5) == 5000000);
	assert(brainwash::DrawingGapMicroseconds(20) == 10000000);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_pattern_covers_canvas_and_switches_layers()
{
	brainwash::Engine engine(42);
	engine.Resize(64, 48);
	assert(engine.Pixels().size() == 64 * 48);
	assert(std::all_of(engine.Pixels().begin(), engine.Pixels().end(),
		[](uint8_t value) { return value == 128; }));

	assert(engine.RenderLines(10000));
	assert(std::all_of(engine.Pixels().begin(), engine.Pixels().end(),
		[](uint8_t value) { return value < 128; }));
	engine.BeginPattern();
	assert(engine.SecondPaletteActive());
	assert(engine.RenderLines(10000));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_palette_cycles_to_visible_colours()
{
	brainwash::Engine engine(7);
	engine.Resize(32, 24);
	for (int32_t index = 0; index < 400; index++)
		engine.AdvancePalette(10);

	assert(std::any_of(engine.Palette().begin(), engine.Palette().begin() + 128,
		[](const brainwash::Colour& colour) {
			return colour.red != 0 || colour.green != 0 || colour.blue != 0;
		}));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_seeded_rendering_is_deterministic()
{
	brainwash::Engine first(1234);
	brainwash::Engine second(1234);
	first.Resize(80, 60);
	second.Resize(80, 60);
	first.RenderLines(75);
	second.RenderLines(75);
	first.AdvancePalette(20);
	second.AdvancePalette(20);
	assert(first.Pixels() == second.Pixels());
	assert(first.Palette() == second.Palette());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_tiny_canvas_is_safe()
{
	brainwash::Engine engine(1);
	engine.Resize(0, 0);
	assert(engine.Width() == 1);
	assert(engine.Height() == 1);
	assert(engine.RenderLines(100));
	assert(engine.Pixels().size() == 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_varied_shapes_and_seeds_finish_safely()
{
	for (uint32_t seed = 0; seed < 64; seed++) {
		brainwash::Engine engine(seed);
		const int32_t width = static_cast<int32_t>(seed % 17) + 1;
		const int32_t height = static_cast<int32_t>((seed * 7) % 23) + 1;
		engine.Resize(width, height);
		assert(engine.RenderLines(10000));
		assert(engine.Pixels().size()
			== static_cast<std::size_t>(width * height));
		assert(std::all_of(engine.Pixels().begin(), engine.Pixels().end(),
			[](uint8_t value) { return value < 128; }));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_setting_conversions_are_bounded();
	test_pattern_covers_canvas_and_switches_layers();
	test_palette_cycles_to_visible_colours();
	test_seeded_rendering_is_deterministic();
	test_tiny_canvas_is_safe();
	test_varied_shapes_and_seeds_finish_safely();
	std::cout << "BrainWash engine tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
