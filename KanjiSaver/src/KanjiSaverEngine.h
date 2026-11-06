#ifndef KANJI_SAVER_ENGINE_H
#define KANJI_SAVER_ENGINE_H

#include <cstdint>
#include <random>
#include <string>


namespace kanjisaver {

constexpr uint32_t kMinimumCodePoint = 0x4e00;
constexpr uint32_t kMaximumCodePoint = 0x9fff;


struct Colour {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	bool operator==(const Colour& other) const;
};


struct Frame {
	uint32_t codePoint;
	Colour colour;
};


double NormaliseHue(double hue);
Colour HslToRgb(double hue, double saturation, double lightness);
std::string Utf8ForCodePoint(uint32_t codePoint);
int32_t ClampIntervalSeconds(int32_t seconds);


class Engine {
public:
	explicit Engine(uint32_t seed);

	Frame NextFrame();

private:
	std::mt19937 fRandomEngine;
};

}


#endif
