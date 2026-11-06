#include "../src/KanjiSaverEngine.h"
#include "../src/TTFXEngine.h"

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
test_hue_normalisation_and_colours()
{
	assert(nearly_equal(kanjisaver::NormaliseHue(-0.25), 0.75));
	assert(nearly_equal(kanjisaver::NormaliseHue(1.5), 0.5));
	const kanjisaver::Colour red = kanjisaver::HslToRgb(0.0, 1.0, 0.5);
	const kanjisaver::Colour green
		= kanjisaver::HslToRgb(1.0 / 3.0, 1.0, 0.5);
	const kanjisaver::Colour blue
		= kanjisaver::HslToRgb(2.0 / 3.0, 1.0, 0.5);
	assert(red.red == 255 && red.green == 0 && red.blue == 0);
	assert(green.red == 0 && green.green == 255 && green.blue == 0);
	assert(blue.red == 0 && blue.green == 0 && blue.blue == 255);
	assert(kanjisaver::HslToRgb(0.7, 0.0, 0.5)
		== (kanjisaver::Colour {128, 128, 128, 255}));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_utf8_encoding()
{
	assert(kanjisaver::Utf8ForCodePoint(0x24) == "$");
	assert(kanjisaver::Utf8ForCodePoint(0xa2) == "\xc2\xa2");
	assert(kanjisaver::Utf8ForCodePoint(0x4e00) == "\xe4\xb8\x80");
	assert(kanjisaver::Utf8ForCodePoint(0x9fff) == "\xe9\xbf\xbf");
	assert(kanjisaver::Utf8ForCodePoint(0x1f642) == "\xf0\x9f\x99\x82");
	assert(kanjisaver::Utf8ForCodePoint(0xd800).empty());
	assert(kanjisaver::Utf8ForCodePoint(0x110000).empty());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_interval_clamping()
{
	assert(kanjisaver::ClampIntervalSeconds(-1) == 1);
	assert(kanjisaver::ClampIntervalSeconds(5) == 5);
	assert(kanjisaver::ClampIntervalSeconds(99) == 15);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_frames_are_deterministic_and_in_range()
{
	kanjisaver::Engine first(42);
	kanjisaver::Engine second(42);
	for (int index = 0; index < 1000; index++) {
		const kanjisaver::Frame firstFrame = first.NextFrame();
		const kanjisaver::Frame secondFrame = second.NextFrame();
		assert(firstFrame.codePoint == secondFrame.codePoint);
		assert(firstFrame.colour == secondFrame.colour);
		assert(firstFrame.codePoint >= kanjisaver::kMinimumCodePoint);
		assert(firstFrame.codePoint <= kanjisaver::kMaximumCodePoint);
		assert(firstFrame.colour.alpha == 255);
		assert(kanjisaver::Utf8ForCodePoint(firstFrame.codePoint).size() == 3);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_ttfx_effects_are_deterministic_and_bounded()
{
	ttfx::EffectEngine first;
	ttfx::EffectEngine second;
	for (std::size_t effectIndex = 0;
		effectIndex < static_cast<std::size_t>(ttfx::Effect::kCount);
		effectIndex++) {
		const ttfx::Effect effect = static_cast<ttfx::Effect>(effectIndex);
		first.SetEffect(effect, 12345);
		second.SetEffect(effect, 12345);
		for (std::size_t glyphIndex = 0; glyphIndex < 16; glyphIndex++) {
			const ttfx::GlyphState firstState = first.Transform(glyphIndex, 16,
				0.6, 0.75, 320.0, 240.0, 1280.0, 720.0);
			const ttfx::GlyphState secondState = second.Transform(glyphIndex, 16,
				0.6, 0.75, 320.0, 240.0, 1280.0, 720.0);
			assert(nearly_equal(firstState.x, secondState.x));
			assert(nearly_equal(firstState.y, secondState.y));
			assert(nearly_equal(firstState.scale, secondState.scale));
			assert(nearly_equal(firstState.opacity, secondState.opacity));
			assert(firstState.opacity >= 0.0 && firstState.opacity <= 1.0);
			assert(firstState.scale > 0.0);
			assert(std::isfinite(firstState.x));
			assert(std::isfinite(firstState.y));
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_hue_normalisation_and_colours();
	test_utf8_encoding();
	test_interval_clamping();
	test_frames_are_deterministic_and_in_range();
	test_ttfx_effects_are_deterministic_and_bounded();
	std::cout << "KanjiSaver engine tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
