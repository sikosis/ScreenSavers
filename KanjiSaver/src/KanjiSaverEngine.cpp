#include "KanjiSaverEngine.h"

#include <algorithm>
#include <cmath>


namespace {

double
hue_channel(double first, double second, double hue)
{
	hue = kanjisaver::NormaliseHue(hue);
	if (hue < 1.0 / 6.0)
		return first + (second - first) * 6.0 * hue;
	if (hue < 1.0 / 2.0)
		return second;
	if (hue < 2.0 / 3.0)
		return first + (second - first) * (2.0 / 3.0 - hue) * 6.0;
	return first;
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint8_t
channel_to_byte(double channel)
{
	channel = std::clamp(channel, 0.0, 1.0);
	return static_cast<uint8_t>(std::lround(channel * 255.0));
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


namespace kanjisaver {

bool
Colour::operator==(const Colour& other) const
{
	return red == other.red && green == other.green && blue == other.blue
		&& alpha == other.alpha;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
NormaliseHue(double hue)
{
	hue = std::fmod(hue, 1.0);
	return hue < 0.0 ? hue + 1.0 : hue;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Colour
HslToRgb(double hue, double saturation, double lightness)
{
	hue = NormaliseHue(hue);
	saturation = std::clamp(saturation, 0.0, 1.0);
	lightness = std::clamp(lightness, 0.0, 1.0);
	if (saturation == 0.0) {
		const uint8_t grey = channel_to_byte(lightness);
		return {grey, grey, grey, 255};
	}

	const double second = lightness <= 0.5
		? lightness * (1.0 + saturation)
		: lightness + saturation - lightness * saturation;
	const double first = 2.0 * lightness - second;
	return {
		channel_to_byte(hue_channel(first, second, hue + 1.0 / 3.0)),
		channel_to_byte(hue_channel(first, second, hue)),
		channel_to_byte(hue_channel(first, second, hue - 1.0 / 3.0)),
		255
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
Utf8ForCodePoint(uint32_t codePoint)
{
	if (codePoint <= 0x7f)
		return std::string(1, static_cast<char>(codePoint));
	if (codePoint <= 0x7ff) {
		std::string text;
		text += static_cast<char>(0xc0 | (codePoint >> 6));
		text += static_cast<char>(0x80 | (codePoint & 0x3f));
		return text;
	}
	if (codePoint >= 0xd800 && codePoint <= 0xdfff)
		return std::string();
	if (codePoint <= 0xffff) {
		std::string text;
		text += static_cast<char>(0xe0 | (codePoint >> 12));
		text += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f));
		text += static_cast<char>(0x80 | (codePoint & 0x3f));
		return text;
	}
	if (codePoint <= 0x10ffff) {
		std::string text;
		text += static_cast<char>(0xf0 | (codePoint >> 18));
		text += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f));
		text += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f));
		text += static_cast<char>(0x80 | (codePoint & 0x3f));
		return text;
	}
	return std::string();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32_t
ClampIntervalSeconds(int32_t seconds)
{
	return std::clamp<int32_t>(seconds, 1, 15);
}
//---------------------------------------------------------------------------------------------------------------------------------//


Engine::Engine(uint32_t seed)
	:
	fRandomEngine(seed)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


Frame
Engine::NextFrame()
{
	std::uniform_int_distribution<uint32_t> codePoint(
		kMinimumCodePoint, kMaximumCodePoint);
	std::uniform_real_distribution<double> hue(0.0, 1.0);
	std::uniform_real_distribution<double> saturation(0.2, 1.0);
	std::uniform_real_distribution<double> lightness(0.2, 1.0);
	const uint32_t nextCodePoint = codePoint(fRandomEngine);
	const double nextHue = hue(fRandomEngine);
	const double nextLightness = lightness(fRandomEngine);
	const double nextSaturation = saturation(fRandomEngine);
	return {
		nextCodePoint,
		HslToRgb(nextHue, nextSaturation, nextLightness)
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
