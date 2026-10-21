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
test_effect_names_and_rotation()
{
	using ttfx::Effect;
	using ttfx::EffectEngine;

	assert(std::strcmp(EffectEngine::EffectName(Effect::kExpand), "Expand") == 0);
	Effect effect = Effect::kExpand;
	for (std::size_t index = 1;
		index < static_cast<std::size_t>(Effect::kCount); index++) {
		effect = EffectEngine::NextEffect(effect);
		assert(effect == static_cast<Effect>(index));
	}
	assert(EffectEngine::NextEffect(effect) == Effect::kExpand);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_completed_effects_reach_the_destination()
{
	ttfx::EffectEngine engine;
	for (std::size_t effect = 0;
		effect < static_cast<std::size_t>(ttfx::Effect::kCount); effect++) {
		engine.SetEffect(static_cast<ttfx::Effect>(effect), 42);
		const ttfx::GlyphState state = engine.Transform(2, 6, 1.0,
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
test_seeded_effects_are_deterministic()
{
	ttfx::EffectEngine first;
	ttfx::EffectEngine second;
	first.SetEffect(ttfx::Effect::kScattered, 8675309);
	second.SetEffect(ttfx::Effect::kScattered, 8675309);
	for (std::size_t index = 0; index < 8; index++) {
		const ttfx::GlyphState firstState = first.Transform(index, 8, 0.35,
			300.0 + index * 40.0, 500.0, 1280.0, 720.0);
		const ttfx::GlyphState secondState = second.Transform(index, 8, 0.35,
			300.0 + index * 40.0, 500.0, 1280.0, 720.0);
		assert(nearly_equal(firstState.x, secondState.x));
		assert(nearly_equal(firstState.y, secondState.y));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_progress_is_clamped()
{
	ttfx::EffectEngine engine;
	engine.SetEffect(ttfx::Effect::kExpand, 1);
	const ttfx::GlyphState before = engine.Transform(0, 1, -10.0,
		100.0, 100.0, 800.0, 600.0);
	const ttfx::GlyphState after = engine.Transform(0, 1, 10.0,
		100.0, 100.0, 800.0, 600.0);
	assert(nearly_equal(before.opacity, 0.0));
	assert(nearly_equal(after.x, 100.0));
	assert(nearly_equal(after.y, 100.0));
	assert(nearly_equal(after.opacity, 1.0));
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_effect_names_and_rotation();
	test_completed_effects_reach_the_destination();
	test_seeded_effects_are_deterministic();
	test_progress_is_clamped();
	std::cout << "ttfx effect engine tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
