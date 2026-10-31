/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef BE_WAKE_ENGINE_H
#define BE_WAKE_ENGINE_H

#include <cstdint>
#include <random>
#include <vector>


namespace bewake {

enum class ColourMode {
	Cycle,
	Random,
	Fixed
};


struct Colour {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	bool operator==(const Colour& other) const;
};


struct WakeLine {
	double x1;
	double y1;
	double x2;
	double y2;
	double hue;
	double saturation;
	double lightness;
};


double NormaliseHue(double hue);
Colour HslToRgb(double hue, double saturation, double lightness);


class Engine {
public:
	explicit Engine(uint32_t seed, int32_t lineCount = 50);

	void Resize(int32_t width, int32_t height);
	void Reset();
	void Step();
	void SetColourMode(ColourMode mode);
	void SetFixedHue(double hue);

	int32_t Width() const;
	int32_t Height() const;
	ColourMode CurrentColourMode() const;
	double FixedHue() const;
	const std::vector<WakeLine>& Lines() const;

private:
	void _MoveCoordinate(double& position, double& velocity, double maximum);
	double _RandomDouble(double minimum, double maximum);

	std::mt19937 fRandomEngine;
	std::vector<WakeLine> fLines;
	int32_t fWidth;
	int32_t fHeight;
	int32_t fStepsSinceReset;
	double fDX1;
	double fDY1;
	double fDX2;
	double fDY2;
	double fFixedHue;
	double fNextHue;
	double fHuePerStep;
	double fLightnessPerStep;
	ColourMode fColourMode;
};

}


#endif
