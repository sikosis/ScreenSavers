#include "../src/TTFXEngine.h"
#include "../src/TTFXTextArt.h"

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
test_default_text_is_block_art()
{
	std::vector<ttfx::TextCell> cells;
	const int32_t columns = ttfx::BuildTextCells("HAIKU", cells);
	assert(columns == 29);
	assert(cells.size() > 60);
	for (const ttfx::TextCell& cell : cells) {
		assert(cell.column >= 0);
		assert(cell.column < columns);
		assert(cell.row >= 0);
		assert(cell.row < 7);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_text_supports_lowercase_spaces_and_fallbacks()
{
	std::vector<ttfx::TextCell> uppercase;
	std::vector<ttfx::TextCell> lowercase;
	assert(ttfx::BuildTextCells("HAIKU", uppercase)
		== ttfx::BuildTextCells("haiku", lowercase));
	assert(uppercase.size() == lowercase.size());

	std::vector<ttfx::TextCell> spaced;
	const int32_t spacedColumns = ttfx::BuildTextCells("HAI KU", spaced);
	assert(spacedColumns == 33);
	assert(spaced.size() == uppercase.size());

	std::vector<ttfx::TextCell> fallback;
	assert(ttfx::BuildTextCells("@", fallback) == 5);
	assert(!fallback.empty());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_all_effects_finish_on_the_text()
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
test_seeded_effect_is_deterministic()
{
	ttfx::EffectEngine first;
	ttfx::EffectEngine second;
	first.SetEffect(ttfx::Effect::kScattered, 8675309);
	second.SetEffect(ttfx::Effect::kScattered, 8675309);
	const ttfx::GlyphState firstState = first.Transform(4, 12, 0.4, 1.2,
		300.0, 500.0, 1280.0, 720.0);
	const ttfx::GlyphState secondState = second.Transform(4, 12, 0.4, 1.2,
		300.0, 500.0, 1280.0, 720.0);
	assert(nearly_equal(firstState.x, secondState.x));
	assert(nearly_equal(firstState.y, secondState.y));
	assert(firstState.colour.red == secondState.colour.red);
	assert(firstState.colour.green == secondState.colour.green);
	assert(firstState.colour.blue == secondState.colour.blue);
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_default_text_is_block_art();
	test_text_supports_lowercase_spaces_and_fallbacks();
	test_all_effects_finish_on_the_text();
	test_seeded_effect_is_deterministic();
	std::cout << "ttfx-text tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
