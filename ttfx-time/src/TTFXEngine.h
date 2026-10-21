#ifndef TTFX_ENGINE_H
#define TTFX_ENGINE_H

#include <cstddef>
#include <cstdint>


namespace ttfx {

enum class Effect {
	kExpand,
	kSlide,
	kWipe,
	kRain,
	kDecrypt,
	kScattered,
	kWaves,
	kColourShift,
	kMatrix,
	kCount
};


struct Colour {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};


struct GlyphState {
	double x;
	double y;
	double scale;
	double opacity;
	Colour colour;
	char replacement;
};


class EffectEngine {
public:
	EffectEngine();

	void SetEffect(Effect effect, uint64_t seed);
	Effect CurrentEffect() const;
	GlyphState Transform(std::size_t index, std::size_t count,
		double progress, double x, double y, double width, double height) const;

	static const char* EffectName(Effect effect);
	static Effect NextEffect(Effect effect);

private:
	static double _Clamp(double value, double minimum, double maximum);
	static double _Ease(double value);
	static uint64_t _Mix(uint64_t value);
	double _Random(std::size_t index, uint64_t salt) const;
	Colour _Gradient(double amount) const;

	Effect fEffect;
	uint64_t fSeed;
};

}


#endif
