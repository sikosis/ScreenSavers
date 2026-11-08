#include "../src/BinaryClockEngine.h"
#include "../src/TTFXEngine.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>


namespace {

bool
nearly_equal(double first, double second)
{
	return std::abs(first - second) < 0.000001;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_24_hour_decoding()
{
	const binaryclock::ClockDigits clock = binaryclock::DecodeClock(13, 7, 9,
		true, true);
	assert(clock.count == 6);
	assert(clock.values[0] == 1);
	assert(clock.values[1] == 3);
	assert(clock.values[2] == 0);
	assert(clock.values[3] == 7);
	assert(clock.values[4] == 0);
	assert(clock.values[5] == 9);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_12_hour_decoding()
{
	assert(binaryclock::DecodeClock(0, 0, 0, true, false).values[0] == 1);
	assert(binaryclock::DecodeClock(0, 0, 0, true, false).values[1] == 2);
	assert(binaryclock::DecodeClock(12, 30, 0, true, false).values[0] == 1);
	assert(binaryclock::DecodeClock(12, 30, 0, true, false).values[1] == 2);
	assert(binaryclock::DecodeClock(13, 30, 0, true, false).values[0] == 0);
	assert(binaryclock::DecodeClock(13, 30, 0, true, false).values[1] == 1);
	assert(binaryclock::DecodeClock(23, 0, 0, true, false).values[0] == 1);
	assert(binaryclock::DecodeClock(23, 0, 0, true, false).values[1] == 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_seconds_toggle()
{
	const binaryclock::ClockDigits withSeconds
		= binaryclock::DecodeClock(12, 34, 56, true, true);
	const binaryclock::ClockDigits withoutSeconds
		= binaryclock::DecodeClock(12, 34, 56, false, true);
	assert(withSeconds.count == 6);
	assert(withoutSeconds.count == 4);
	assert(withoutSeconds.values[0] == 1);
	assert(withoutSeconds.values[1] == 2);
	assert(withoutSeconds.values[2] == 3);
	assert(withoutSeconds.values[3] == 4);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_input_is_clamped()
{
	const binaryclock::ClockDigits clock = binaryclock::DecodeClock(99, -5, 100,
		true, true);
	assert(clock.values[0] == 2);
	assert(clock.values[1] == 3);
	assert(clock.values[2] == 0);
	assert(clock.values[3] == 0);
	assert(clock.values[4] == 5);
	assert(clock.values[5] == 9);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_bit_cells_and_weights()
{
	const binaryclock::ClockDigits clock = binaryclock::DecodeClock(12, 34, 56,
		true, true);
	std::vector<binaryclock::BitCell> cells;
	const int total = binaryclock::BuildBitCells(clock, cells);
	assert(total == 24);
	assert(cells.size() == 24);

	for (const binaryclock::BitCell& cell : cells) {
		assert(cell.column >= 0);
		assert(cell.column < 6);
		assert(cell.row >= 0);
		assert(cell.row < 4);
		assert(cell.weight == 1 || cell.weight == 2 || cell.weight == 4
			|| cell.weight == 8);
		if (cell.column == 4 && cell.row == 1)
			assert(cell.weight == 4 && cell.lit);
		if (cell.column == 4 && cell.row == 3)
			assert(cell.weight == 1 && cell.lit);
		if (cell.column == 4 && cell.row == 0)
			assert(!cell.lit);
	}
	assert(binaryclock::LitBitCount(cells) == 9);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_zero_digit_has_no_lit_bits()
{
	const binaryclock::ClockDigits clock = binaryclock::DecodeClock(1, 2, 3,
		true, true);
	std::vector<binaryclock::BitCell> cells;
	binaryclock::BuildBitCells(clock, cells);
	for (const binaryclock::BitCell& cell : cells) {
		if (cell.column == 0 || cell.column == 2 || cell.column == 4)
			assert(!cell.lit);
	}
	assert(binaryclock::LitBitCount(cells) == 4);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_column_positions_and_groups()
{
	assert(nearly_equal(binaryclock::ColumnPosition(0), 0.0));
	assert(nearly_equal(binaryclock::ColumnPosition(1), 1.0));
	assert(nearly_equal(binaryclock::ColumnPosition(2), 2.9));
	assert(nearly_equal(binaryclock::ColumnPosition(3), 3.9));
	assert(nearly_equal(binaryclock::ColumnPosition(4), 5.8));
	assert(nearly_equal(binaryclock::ColumnPosition(5), 6.8));

	assert(binaryclock::ColumnGroup(0) == 0);
	assert(binaryclock::ColumnGroup(1) == 0);
	assert(binaryclock::ColumnGroup(2) == 1);
	assert(binaryclock::ColumnGroup(3) == 1);
	assert(binaryclock::ColumnGroup(4) == 2);
	assert(binaryclock::ColumnGroup(5) == 2);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_effects_complete_at_their_destination()
{
	ttfx::EffectEngine engine;
	for (std::size_t effect = 0;
		effect < static_cast<std::size_t>(ttfx::Effect::kCount); effect++) {
		engine.SetEffect(static_cast<ttfx::Effect>(effect), 42);
		const ttfx::GlyphState state = engine.Transform(2, 6, 1.0, 3.0,
			120.0, 240.0, 1920.0, 1080.0);
		assert(nearly_equal(state.x, 120.0));
		assert(nearly_equal(state.y, 240.0));
		assert(nearly_equal(state.scale, 1.0));
		assert(nearly_equal(state.opacity, 1.0));
		assert(state.replacement == '\0');
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_effect_rotation_reaches_every_effect()
{
	using ttfx::Effect;
	using ttfx::EffectEngine;

	Effect effect = Effect::kExpand;
	for (std::size_t index = 1;
		index < static_cast<std::size_t>(Effect::kCount); index++) {
		effect = EffectEngine::NextEffect(effect);
		assert(effect == static_cast<Effect>(index));
	}
	assert(EffectEngine::NextEffect(effect) == Effect::kExpand);
	assert(std::strcmp(EffectEngine::EffectName(Effect::kExpand),
		"Expand") == 0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_24_hour_decoding();
	test_12_hour_decoding();
	test_seconds_toggle();
	test_input_is_clamped();
	test_bit_cells_and_weights();
	test_zero_digit_has_no_lit_bits();
	test_column_positions_and_groups();
	test_effects_complete_at_their_destination();
	test_effect_rotation_reaches_every_effect();
	std::cout << "BinaryClock engine tests passed\n";
	return 0;
}
