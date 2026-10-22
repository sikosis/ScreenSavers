#include "TTFXEngine.h"

#include <algorithm>
#include <cmath>


namespace ttfx {

namespace {

constexpr double kPi = 3.14159265358979323846;


uint8_t
interpolate_channel(uint8_t first, uint8_t second, double amount)
{
	return static_cast<uint8_t>(std::lround(first
		+ (second - first) * amount));
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


EffectEngine::EffectEngine()
	:
	fEffect(Effect::kExpand),
	fSeed(0)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
EffectEngine::SetEffect(Effect effect, uint64_t seed)
{
	fEffect = effect < Effect::kCount ? effect : Effect::kExpand;
	fSeed = seed;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Effect
EffectEngine::CurrentEffect() const
{
	return fEffect;
}
//---------------------------------------------------------------------------------------------------------------------------------//


GlyphState
EffectEngine::Transform(std::size_t index, std::size_t count, double progress,
	double animationTime, double x, double y, double width, double height) const
{
	progress = _Clamp(progress, 0.0, 1.0);
	const double position = count > 1
		? static_cast<double>(index) / static_cast<double>(count - 1) : 0.5;
	GlyphState state = {x, y, 1.0, 1.0, _Gradient(position), '\0'};

	switch (fEffect) {
		case Effect::kExpand: {
			const double delay = position * 0.12;
			const double local = _Ease(_Clamp((progress - delay) / 0.88, 0.0, 1.0));
			state.x = width / 2.0 + (x - width / 2.0) * local;
			state.y = height / 2.0 + (y - height / 2.0) * local;
			state.scale = 0.2 + local * 0.8;
			state.opacity = local;
			break;
		}

		case Effect::kSlide: {
			const double delay = position * 0.25;
			const double local = _Ease(_Clamp((progress - delay) / 0.75, 0.0, 1.0));
			state.x = -width * 0.12 + (x + width * 0.12) * local;
			state.y = y + std::sin(position * 4.0 * kPi) * height * 0.035
				* (1.0 - local);
			state.opacity = local;
			break;
		}

		case Effect::kWipe: {
			const double local = _Clamp(progress * 1.55 - position * 0.55, 0.0, 1.0);
			state.opacity = _Ease(local);
			state.scale = 0.85 + state.opacity * 0.15;
			break;
		}

		case Effect::kRain: {
			const double delay = _Random(index, 1) * 0.32;
			const double raw = _Clamp((progress - delay) / 0.68, 0.0, 1.0);
			const double local = _Ease(raw);
			const double startY = -height * (0.08 + _Random(index, 2) * 0.48);
			state.y = startY + (y - startY) * local
				+ std::sin(raw * kPi) * height * 0.025;
			state.opacity = _Clamp(raw * 2.2, 0.0, 1.0);
			break;
		}

		case Effect::kDecrypt: {
			const double threshold = _Random(index, 3) * 0.55 + 0.35;
			state.opacity = _Clamp(progress * 2.5, 0.0, 1.0);
			if (progress < threshold) {
				static const char symbols[] = "0123456789ABCDEF#$%&@";
				const uint64_t frame = static_cast<uint64_t>(animationTime * 18.0);
				state.replacement = symbols[_Mix(fSeed + index * 37 + frame)
					% (sizeof(symbols) - 1)];
				state.colour = {67, 255, 153};
			}
			break;
		}

		case Effect::kScattered: {
			const double eased = _Ease(progress);
			const double startX = _Random(index, 4) * width;
			const double startY = _Random(index, 5) * height;
			state.x = startX + (x - startX) * eased;
			state.y = startY + (y - startY) * eased;
			state.opacity = _Clamp(progress * 2.0, 0.0, 1.0);
			break;
		}

		case Effect::kWaves: {
			const double local = _Clamp(progress * 1.35 - position * 0.35, 0.0, 1.0);
			state.y = y + std::sin((position * 3.0 - animationTime * 0.8)
				* 2.0 * kPi) * height * 0.065 * (1.0 - local);
			state.opacity = _Clamp(local * 2.0, 0.0, 1.0);
			break;
		}

		case Effect::kColourShift: {
			const double shift = std::fmod(position + animationTime * 0.16, 1.0);
			state.colour = _Gradient(shift);
			state.opacity = _Clamp(progress * 3.0, 0.0, 1.0);
			break;
		}

		case Effect::kMatrix: {
			const double delay = _Random(index, 6) * 0.5;
			const double local = _Clamp((progress - delay) / 0.5, 0.0, 1.0);
			state.y = y - (1.0 - _Ease(local)) * height
				* (0.2 + _Random(index, 7) * 0.55);
			state.opacity = _Clamp(local * 2.0, 0.0, 1.0);
			state.colour = local < 0.75 ? Colour{48, 255, 112} : _Gradient(position);
			if (local < 0.88) {
				const uint64_t frame = static_cast<uint64_t>(animationTime * 20.0);
				state.replacement = ((_Mix(fSeed + index * 53 + frame) & 1) == 0)
					? '0' : '1';
			}
			break;
		}

		case Effect::kCount:
			break;
	}

	return state;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const char*
EffectEngine::EffectName(Effect effect)
{
	static const char* const names[] = {
		"Expand", "Slide", "Wipe", "Rain", "Decrypt", "Scattered",
		"Waves", "Colour shift", "Matrix"
	};
	const std::size_t index = static_cast<std::size_t>(effect);
	return index < static_cast<std::size_t>(Effect::kCount) ? names[index] : "Expand";
}
//---------------------------------------------------------------------------------------------------------------------------------//


Effect
EffectEngine::NextEffect(Effect effect)
{
	const std::size_t next = (static_cast<std::size_t>(effect) + 1)
		% static_cast<std::size_t>(Effect::kCount);
	return static_cast<Effect>(next);
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
EffectEngine::_Clamp(double value, double minimum, double maximum)
{
	return std::max(minimum, std::min(maximum, value));
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
EffectEngine::_Ease(double value)
{
	value = _Clamp(value, 0.0, 1.0);
	return value < 0.5 ? 2.0 * value * value
		: 1.0 - std::pow(-2.0 * value + 2.0, 2.0) / 2.0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint64_t
EffectEngine::_Mix(uint64_t value)
{
	value += UINT64_C(0x9e3779b97f4a7c15);
	value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
	value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
	return value ^ (value >> 31);
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
EffectEngine::_Random(std::size_t index, uint64_t salt) const
{
	return static_cast<double>(_Mix(fSeed + index * UINT64_C(0x9e3779b97f4a7c15)
		+ salt) >> 11) * (1.0 / static_cast<double>(UINT64_C(1) << 53));
}
//---------------------------------------------------------------------------------------------------------------------------------//


Colour
EffectEngine::_Gradient(double amount) const
{
	static const Colour palettes[][3] = {
		{{138, 43, 226}, {0, 209, 255}, {255, 255, 255}},
		{{255, 72, 120}, {255, 176, 69}, {255, 246, 189}},
		{{35, 210, 92}, {62, 255, 213}, {218, 255, 241}},
		{{72, 125, 231}, {171, 157, 255}, {255, 209, 243}},
		{{255, 75, 43}, {255, 210, 63}, {255, 255, 224}},
		{{0, 174, 255}, {127, 255, 212}, {240, 255, 255}}
	};
	const std::size_t paletteCount = sizeof(palettes) / sizeof(palettes[0]);
	const Colour* palette = palettes[_Mix(fSeed) % paletteCount];
	if ((_Mix(fSeed + 1) & 1) != 0)
		amount = 1.0 - amount;
	amount = _Clamp(amount, 0.0, 1.0);
	if (amount <= 0.5) {
		const double local = amount * 2.0;
		return {
			interpolate_channel(palette[0].red, palette[1].red, local),
			interpolate_channel(palette[0].green, palette[1].green, local),
			interpolate_channel(palette[0].blue, palette[1].blue, local)
		};
	}

	const double local = (amount - 0.5) * 2.0;
	return {
		interpolate_channel(palette[1].red, palette[2].red, local),
		interpolate_channel(palette[1].green, palette[2].green, local),
		interpolate_channel(palette[1].blue, palette[2].blue, local)
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
