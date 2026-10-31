/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#include "BeWakeEngine.h"

#include <algorithm>
#include <cmath>


namespace {

double
HueComponent(double first, double second, double hue)
{
	hue = bewake::NormaliseHue(hue);
	if (hue < 1.0 / 6.0)
		return first + (second - first) * 6.0 * hue;
	if (hue < 0.5)
		return second;
	if (hue < 2.0 / 3.0)
		return first + (second - first) * (2.0 / 3.0 - hue) * 6.0;
	return first;
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint8_t
ColourChannel(double value)
{
	return static_cast<uint8_t>(std::clamp(value, 0.0, 1.0) * 255.0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


namespace bewake {

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
	if (hue < 0.0)
		hue += 1.0;
	return hue;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Colour
HslToRgb(double hue, double saturation, double lightness)
{
	hue = NormaliseHue(hue);
	saturation = std::clamp(saturation, 0.0, 1.0);
	lightness = std::clamp(lightness, 0.0, 1.0);
	if (saturation == 0.0) {
		const uint8_t channel = ColourChannel(lightness);
		return { channel, channel, channel, 255 };
	}

	const double second = lightness <= 0.5
		? lightness * (1.0 + saturation)
		: lightness + saturation - lightness * saturation;
	const double first = 2.0 * lightness - second;
	return {
		ColourChannel(HueComponent(first, second, hue + 1.0 / 3.0)),
		ColourChannel(HueComponent(first, second, hue)),
		ColourChannel(HueComponent(first, second, hue - 1.0 / 3.0)),
		255
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


Engine::Engine(uint32_t seed, int32_t lineCount)
	:
	fRandomEngine(seed),
	fLines(static_cast<std::size_t>(std::max(lineCount, 1))),
	fWidth(1),
	fHeight(1),
	fStepsSinceReset(0),
	fDX1(3.0),
	fDY1(3.0),
	fDX2(4.0),
	fDY2(4.0),
	fFixedHue(0.66),
	fNextHue(0.66),
	fHuePerStep(0.01),
	fLightnessPerStep(0.01),
	fColourMode(ColourMode::Cycle)
{
	Reset();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::Resize(int32_t width, int32_t height)
{
	fWidth = std::max(width, 1);
	fHeight = std::max(height, 1);
	Reset();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::Reset()
{
	fLightnessPerStep = 0.5 / fLines.size();
	fHuePerStep = 0.01;
	if (fColourMode == ColourMode::Fixed)
		fNextHue = fFixedHue;
	else
		fNextHue = _RandomDouble(0.0, 1.0);

	const double x1 = _RandomDouble(0.0, fWidth - 1.0);
	const double y1 = _RandomDouble(0.0, fHeight - 1.0);
	const double x2 = _RandomDouble(0.0, fWidth - 1.0);
	const double y2 = _RandomDouble(0.0, fHeight - 1.0);
	for (std::size_t index = 0; index < fLines.size(); index++) {
		fLines[index] = { x1, y1, x2, y2, fNextHue, 1.0,
			index == 0 ? 0.5 : 0.0 };
	}
	fStepsSinceReset = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::Step()
{
	if (fColourMode == ColourMode::Random && fStepsSinceReset >= 1000)
		Reset();

	double x1 = fLines[0].x1;
	double y1 = fLines[0].y1;
	double x2 = fLines[0].x2;
	double y2 = fLines[0].y2;
	_MoveCoordinate(x1, fDX1, fWidth - 1.0);
	_MoveCoordinate(y1, fDY1, fHeight - 1.0);
	_MoveCoordinate(x2, fDX2, fWidth - 1.0);
	_MoveCoordinate(y2, fDY2, fHeight - 1.0);

	for (std::size_t index = fLines.size() - 1; index > 0; index--) {
		fLines[index] = fLines[index - 1];
		fLines[index].lightness = std::max(
			fLines[index - 1].lightness - fLightnessPerStep, 0.0);
	}

	if (fColourMode == ColourMode::Cycle)
		fNextHue = NormaliseHue(fNextHue + fHuePerStep);
	fLines[0] = { x1, y1, x2, y2, fNextHue, 1.0, 0.5 };
	fStepsSinceReset++;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::SetColourMode(ColourMode mode)
{
	if (mode == fColourMode)
		return;
	fColourMode = mode;
	Reset();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::SetFixedHue(double hue)
{
	fFixedHue = NormaliseHue(hue);
	if (fColourMode == ColourMode::Fixed)
		fNextHue = fFixedHue;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32_t
Engine::Width() const
{
	return fWidth;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32_t
Engine::Height() const
{
	return fHeight;
}
//---------------------------------------------------------------------------------------------------------------------------------//


ColourMode
Engine::CurrentColourMode() const
{
	return fColourMode;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
Engine::FixedHue() const
{
	return fFixedHue;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::vector<WakeLine>&
Engine::Lines() const
{
	return fLines;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::_MoveCoordinate(double& position, double& velocity, double maximum)
{
	if (maximum <= 0.0) {
		position = 0.0;
		return;
	}

	position += velocity;
	while (position < 0.0 || position > maximum) {
		if (position > maximum) {
			position = maximum - (position - maximum);
			velocity = -std::abs(velocity);
		} else {
			position = -position;
			velocity = std::abs(velocity);
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
Engine::_RandomDouble(double minimum, double maximum)
{
	if (maximum <= minimum)
		return minimum;
	std::uniform_real_distribution<double> distribution(minimum, maximum);
	return distribution(fRandomEngine);
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
