/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#include "BrainWashEngine.h"

#include <algorithm>
#include <cmath>
#include <cstddef>


namespace {

const double kPi = 3.14159265358979323846;
const double kTwoPi = kPi * 2.0;
const double kHalfPi = kPi / 2.0;


double
LegacyAngle(double deltaX, double deltaY, double distance)
{
	double angle = kHalfPi;
	if (distance > 0.0) {
		const double ratio = std::clamp(deltaY / (distance * 1.000001),
			-1.0, 1.0);
		angle = std::asin(ratio);
	}
	if (deltaX > 0.0)
		angle = kPi - angle;
	return angle;
}
//---------------------------------------------------------------------------------------------------------------------------------//


brainwash::Colour
ScaledColour(const brainwash::Colour& colour, int32_t value)
{
	const double factor = static_cast<double>(value) / 63.0;
	return {
		static_cast<uint8_t>(colour.red * factor),
		static_cast<uint8_t>(colour.green * factor),
		static_cast<uint8_t>(colour.blue * factor),
		255
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


namespace brainwash {

bool
Colour::operator==(const Colour& other) const
{
	return red == other.red && green == other.green && blue == other.blue
		&& alpha == other.alpha;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32_t
DrawingLinesPerFrame(int32_t drawingSpeed)
{
	drawingSpeed = std::clamp(drawingSpeed, 1, 9);
	const int32_t divisor = 10 - drawingSpeed;
	return std::max<int32_t>((67 + divisor / 2) / divisor, 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


int64_t
DrawingGapMicroseconds(int32_t drawingGap)
{
	return static_cast<int64_t>(std::clamp(drawingGap, 0, 10)) * 1000000;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Engine::Engine(uint32_t seed)
	:
	fRandomEngine(seed),
	fWidth(1),
	fHeight(1),
	fPaletteOffset(0),
	fCurrentLayer(0),
	fNextLayer(0),
	fLeft(0),
	fTop(0),
	fRight(0),
	fBottom(0),
	fSide(0),
	fPatternComplete(true),
	fSecondPaletteActive(false)
{
	Resize(1, 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::Resize(int32_t width, int32_t height)
{
	fWidth = std::max<int32_t>(width, 1);
	fHeight = std::max<int32_t>(height, 1);
	fPixels.assign(static_cast<std::size_t>(fWidth) * fHeight, 128);
	Reset();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::Reset()
{
	const Colour black = { 0, 0, 0, 255 };
	fPalette.fill(black);
	fSourceColours.fill(black);
	fDestinationColours.fill(black);
	fCurrentColours.fill(black);
	fColourSteps.fill(0);
	fCurrentColourSteps.fill(0);
	fPaletteOffset = 0;
	fCurrentLayer = 0;
	fNextLayer = 0;
	fSecondPaletteActive = false;
	std::fill(fPixels.begin(), fPixels.end(), 128);
	_BuildPalette();
	BeginPattern();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::BeginPattern()
{
	fCurrentLayer = fNextLayer;
	fNextLayer = 128 - fNextLayer;
	if (fCurrentLayer == 128)
		fSecondPaletteActive = true;

	_CreateTransforms();
	const int32_t minimumX = fWidth / 4;
	const int32_t minimumY = fHeight / 4;
	const int32_t centreX = minimumX + _RandomInt(std::max(fWidth / 2, 1));
	const int32_t centreY = minimumY + _RandomInt(std::max(fHeight / 2, 1));
	fLeft = std::clamp(centreX, 0, fWidth - 1) - 1;
	fRight = fLeft + 1;
	fTop = std::clamp(centreY, 0, fHeight - 1) - 1;
	fBottom = fTop + 2;
	fSide = 0;
	fPatternComplete = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
Engine::RenderLines(int32_t maximumLines)
{
	if (fPatternComplete || maximumLines <= 0)
		return fPatternComplete;

	std::vector<Point> points;
	std::vector<Point> transformedPoints;
	for (int32_t line = 0; line < maximumLines; line++) {
		if (!_NextLine(points)) {
			fPatternComplete = true;
			break;
		}

		transformedPoints = points;
		for (const Transform& transform : fPreTransforms) {
			for (Point& point : transformedPoints)
				point = _ApplyPreTransform(point, transform);
		}

		for (std::size_t index = 0; index < points.size(); index++) {
			double result = 0.0;
			for (const Transform& transform : fPostTransforms)
				result += _ApplyPostTransform(transformedPoints[index], transform);
			if (!std::isfinite(result))
				result = 0.0;

			int32_t colour = static_cast<int32_t>(std::floor(result * 64.0 + 0.5));
			colour = ((colour % 128) + 128) % 128;
			const int32_t x = static_cast<int32_t>(points[index].x);
			const int32_t y = static_cast<int32_t>(points[index].y);
			if (x >= 0 && x < fWidth && y >= 0 && y < fHeight) {
				fPixels[static_cast<std::size_t>(y) * fWidth + x]
					= static_cast<uint8_t>(colour + fCurrentLayer);
			}
		}
	}

	return fPatternComplete;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::AdvancePalette(int32_t steps)
{
	steps = std::clamp<int32_t>(steps, 1, 32);
	for (int32_t advance = 0; advance < steps; advance++) {
		if (fPaletteOffset % 16 == 0) {
			for (int32_t paletteIndex = 0; paletteIndex < 2; paletteIndex++) {
				if (fCurrentColourSteps[paletteIndex]
					== fColourSteps[paletteIndex]) {
					_StartPaletteTransition(paletteIndex);
				}
				fCurrentColours[paletteIndex] = _ColourBetween(
					fSourceColours[paletteIndex], fDestinationColours[paletteIndex],
					fCurrentColourSteps[paletteIndex], fColourSteps[paletteIndex]);
				fCurrentColourSteps[paletteIndex]++;
			}
		}

		_BuildPalette();
		fPaletteOffset = (fPaletteOffset + 1) % 128;
	}
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


bool
Engine::PatternComplete() const
{
	return fPatternComplete;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
Engine::SecondPaletteActive() const
{
	return fSecondPaletteActive;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::vector<uint8_t>&
Engine::Pixels() const
{
	return fPixels;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::array<Colour, 256>&
Engine::Palette() const
{
	return fPalette;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::_BuildPalette()
{
	const Colour black = { 0, 0, 0, 255 };
	fPalette.fill(black);
	for (int32_t index = 0; index < 64; index++) {
		const Colour colour = ScaledColour(fCurrentColours[0], index);
		fPalette[(fPaletteOffset + 127 - index) % 128] = colour;
		fPalette[(fPaletteOffset + index) % 128] = colour;
	}

	if (!fSecondPaletteActive)
		return;

	for (int32_t index = 0; index < 64; index++) {
		const Colour colour = ScaledColour(fCurrentColours[1], index);
		fPalette[(fPaletteOffset + 127 - index) % 128 + 128] = colour;
		fPalette[(fPaletteOffset + index) % 128 + 128] = colour;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


Colour
Engine::_ColourBetween(const Colour& source, const Colour& destination,
	int32_t step, int32_t steps) const
{
	if (steps <= 0)
		return destination;
	step = std::clamp(step, 0, steps);
	return {
		static_cast<uint8_t>((static_cast<int32_t>(destination.red) - source.red)
			* step / steps + source.red),
		static_cast<uint8_t>((static_cast<int32_t>(destination.green) - source.green)
			* step / steps + source.green),
		static_cast<uint8_t>((static_cast<int32_t>(destination.blue) - source.blue)
			* step / steps + source.blue),
		255
	};
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::_CreateTransforms()
{
	fPreTransforms.clear();
	fPostTransforms.clear();

	std::array<PreKind, 3> preKinds = {
		PreKind::Cosine, PreKind::Lens, PreKind::Twist
	};
	std::shuffle(preKinds.begin(), preKinds.end(), fRandomEngine);
	const int32_t preCount = _RandomInt(2) + 1;
	for (int32_t index = 0; index < preCount; index++) {
		Transform transform = {};
		transform.kind = static_cast<int32_t>(preKinds[index]);
		transform.x0 = _RandomDouble(0.0, fWidth - 1.0);
		transform.y0 = _RandomDouble(0.0, fHeight - 1.0);
		if (preKinds[index] == PreKind::Lens) {
			transform.first = _RandomDouble(0.3, 3.0);
			transform.second = _RandomDouble(-1.5, 1.5);
			transform.third = _MaximumRadius(transform.x0, transform.y0);
		} else if (preKinds[index] == PreKind::Twist) {
			transform.first = _RandomDouble(1.0, 10.0);
			transform.second = (_RandomInt(2) == 0 ? -1.0 : 1.0)
				* _RandomDouble(0.1, 2.0);
		}
		fPreTransforms.push_back(transform);
	}

	const int32_t postCount = _RandomInt(4) + 2;
	int32_t spiralRemaining = 2;
	int32_t circleRemaining = 2;
	int32_t gridOrFieldRemaining = 1;
	bool snailAvailable = true;
	bool wavesAvailable = true;
	bool holeAvailable = true;
	while (static_cast<int32_t>(fPostTransforms.size()) < postCount) {
		const int32_t category = _RandomInt(7);
		Transform transform = {};
		transform.x0 = _RandomDouble(0.0, fWidth - 1.0);
		transform.y0 = _RandomDouble(0.0, fHeight - 1.0);
		bool accepted = true;
		switch (category) {
			case 0:
				if (spiralRemaining <= 0) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::SpiralCosine)
					+ _RandomInt(6);
				transform.first = (_RandomInt(2) == 0 ? -1.0 : 1.0)
					* (_RandomInt(19) + 4);
				transform.second = _RandomDouble(1.0, 10.0);
				spiralRemaining--;
				break;
			case 1:
				if (!snailAvailable) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::Snail);
				transform.first = (_RandomInt(2) == 0 ? -1.0 : 1.0)
					* _RandomDouble(0.1, 2.0);
				snailAvailable = false;
				break;
			case 2:
				if (circleRemaining <= 0) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(_RandomInt(2) == 0
					? PostKind::CircleCosine : PostKind::CircleLinear);
				transform.first = (_RandomInt(2) == 0 ? -1.0 : 1.0)
					* _RandomDouble(0.5, 15.0);
				transform.third = _MaximumRadius(transform.x0, transform.y0);
				circleRemaining--;
				break;
			case 3:
				if (gridOrFieldRemaining <= 0) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::Grid);
				transform.first = _RandomDouble(15.0, 400.0);
				transform.second = _RandomDouble(15.0, 400.0);
				transform.third = _RandomDouble(0.05, 0.55);
				gridOrFieldRemaining--;
				break;
			case 4:
				if (!wavesAvailable) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::Waves);
				transform.first = (_RandomInt(2) == 0 ? -1.0 : 1.0)
					* _RandomDouble(1.0, 20.0);
				transform.second = (_RandomInt(2) == 0 ? -1.0 : 1.0)
					* _RandomDouble(1.0, 20.0);
				wavesAvailable = false;
				break;
			case 5:
				if (gridOrFieldRemaining <= 0) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::Field);
				transform.first = _RandomDouble(1.0, 30.0);
				transform.second = _RandomDouble(1.0, 30.0);
				transform.third = _RandomDouble(0.05, 0.55);
				gridOrFieldRemaining--;
				break;
			default:
				if (!holeAvailable) {
					accepted = false;
					break;
				}
				transform.kind = static_cast<int32_t>(PostKind::Hole);
				transform.first = _RandomInt(5) + 1;
				transform.second = _RandomInt(2) == 0 ? -1.0 : 1.0;
				transform.third = _MaximumRadius(transform.x0, transform.y0);
				holeAvailable = false;
				break;
		}
		if (accepted)
			fPostTransforms.push_back(transform);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


Colour
Engine::_NewDestinationColour()
{
	const int32_t minimumTotal = 382;
	Colour colour = { 0, 0, 0, 255 };
	colour.red = static_cast<uint8_t>(_RandomInt(256));
	int32_t lowerBound = std::max(minimumTotal - 255 - colour.red, 0);
	colour.green = static_cast<uint8_t>(lowerBound
		+ _RandomInt(256 - lowerBound));
	lowerBound = std::max(minimumTotal - colour.red - colour.green, 0);
	colour.blue = static_cast<uint8_t>(lowerBound
		+ _RandomInt(256 - lowerBound));
	return colour;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
Engine::_NextLine(std::vector<Point>& points)
{
	points.clear();
	if (!(fTop >= 0 || fRight < fWidth || fBottom < fHeight || fLeft >= 0))
		return false;

	bool horizontal = false;
	bool reverse = false;
	int32_t x = 0;
	int32_t y = 0;
	int32_t count = 0;
	while (count <= 0) {
		fSide = (fSide + 1) % 4;
		switch (fSide) {
			case 0:
				if (fTop >= 0) {
					x = fLeft + 1;
					y = fTop;
					count = fRight - fLeft - 1;
					horizontal = true;
					fTop--;
				}
				break;
			case 1:
				if (fRight < fWidth) {
					x = fRight;
					y = fTop + 1;
					count = fBottom - fTop - 1;
					fRight++;
				}
				break;
			case 2:
				if (fBottom < fHeight) {
					x = fLeft + 1;
					y = fBottom;
					count = fRight - fLeft - 1;
					horizontal = true;
					reverse = true;
					fBottom++;
				}
				break;
			default:
				if (fLeft >= 0) {
					x = fLeft;
					y = fTop + 1;
					count = fBottom - fTop - 1;
					reverse = true;
					fLeft--;
				}
				break;
		}
	}

	points.reserve(static_cast<std::size_t>(count));
	for (int32_t index = 0; index < count; index++) {
		if (horizontal) {
			const int32_t pointX = reverse ? x + count - index - 1 : x + index;
			points.push_back({ static_cast<double>(pointX), static_cast<double>(y) });
		} else {
			const int32_t pointY = reverse ? y + count - index - 1 : y + index;
			points.push_back({ static_cast<double>(x), static_cast<double>(pointY) });
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Point
Engine::_ApplyPreTransform(Point point, const Transform& transform) const
{
	const PreKind kind = static_cast<PreKind>(transform.kind);
	if (kind == PreKind::Cosine) {
		point.x = (1.0 - std::cos(point.x / fWidth * kPi)) * 0.5 * fWidth;
		point.y = (1.0 - std::cos(point.y / fHeight * kPi)) * 0.5 * fHeight;
		return point;
	}

	const double deltaX = transform.x0 - point.x;
	const double deltaY = transform.y0 - point.y;
	const double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
	if (kind == PreKind::Lens) {
		const double radius = std::pow(std::abs(std::cos(distance
			/ (transform.third + 0.01) * kHalfPi)), transform.first)
			* transform.second;
		point.x += deltaX * radius;
		point.y += deltaY * radius;
		return point;
	}

	double angle = LegacyAngle(deltaX, deltaY, distance);
	angle += std::sqrt(distance / 1120.0) * kTwoPi;
	point.x = transform.x0 + std::cos(angle) * distance;
	point.y = transform.y0 + std::sin(angle) * distance;
	return point;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
Engine::_ApplyPostTransform(const Point& point, const Transform& transform) const
{
	const PostKind kind = static_cast<PostKind>(transform.kind);
	const double deltaX = transform.x0 - point.x;
	const double deltaY = transform.y0 - point.y;
	const double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

	switch (kind) {
		case PostKind::SpiralCosine:
		case PostKind::SpiralCosineInverse:
		case PostKind::SpiralLinear:
		case PostKind::SpiralLinearInverse: {
			double angle = LegacyAngle(deltaX, deltaY, distance);
			const bool inverse = kind == PostKind::SpiralCosineInverse
				|| kind == PostKind::SpiralLinearInverse;
			const double curve = inverse
				? std::sqrt(transform.second / (transform.second + distance)) * 3.0
				: std::sqrt(distance / (fWidth + fHeight)) * 3.0;
			double value = (angle + curve) / kTwoPi * transform.first;
			if (kind == PostKind::SpiralLinear
				|| kind == PostKind::SpiralLinearInverse) {
				return (value - std::floor(value) - 0.5) * 1.999;
			}
			value -= std::floor(value);
			const double magnitude = (std::cos(value * kTwoPi) + 1.0) * 0.4999;
			return value < 0.5 ? -magnitude : magnitude;
		}
		case PostKind::WheelCosine:
		case PostKind::WheelLinear: {
			double value = LegacyAngle(deltaX, deltaY, distance) / kTwoPi
				* transform.first;
			if (kind == PostKind::WheelLinear)
				return 1.0 - 2.0 * (value - std::floor(value));
			value -= std::floor(value);
			const double magnitude = (std::cos(value * kTwoPi) + 1.0) * 0.4999;
			return value < 0.5 ? -magnitude : magnitude;
		}
		case PostKind::Snail: {
			double value = (LegacyAngle(deltaX, deltaY, distance)
				+ std::sqrt(distance / (fWidth + fHeight)) * 100.0
					* transform.first) / kTwoPi;
			return (value - std::floor(value) - 0.5) * 1.999;
		}
		case PostKind::Grid: {
			double horizontal = (point.x - transform.x0) / transform.first;
			double vertical = (point.y - transform.y0) / transform.second;
			horizontal -= std::floor(horizontal);
			vertical -= std::floor(vertical);
			if (horizontal > 0.5)
				horizontal = 1.0 - horizontal;
			if (vertical > 0.5)
				vertical = 1.0 - vertical;
			horizontal = std::sin(kTwoPi * horizontal);
			vertical = std::sin(kTwoPi * vertical);
			return (1.0 - horizontal * vertical) * transform.third;
		}
		case PostKind::Waves: {
			const double value = ((point.x - transform.x0) * transform.first
				+ (point.y - transform.y0) * transform.second)
				/ (fWidth + fHeight);
			return (value - 0.5) * 1.999;
		}
		case PostKind::Field:
			return (std::cos(point.x * transform.first / fWidth * kPi)
				+ std::cos(point.y * transform.second / fHeight * kPi) + 2.0)
				/ 4.0 * transform.third;
		case PostKind::CircleCosine:
		case PostKind::CircleLinear: {
			double value = distance / (transform.third + 0.1) * transform.first;
			if (kind == PostKind::CircleLinear)
				return (value - std::floor(value) - 0.5) * 1.999;
			value -= std::floor(value);
			const double magnitude = (std::cos(value * kTwoPi) + 1.0) * 0.4999;
			return value < 0.5 ? -magnitude : magnitude;
		}
		case PostKind::Hole: {
			double value = distance / (transform.third + 0.1) * transform.first;
			value = transform.second * (0.1 / (value + 0.1));
			return (value - std::floor(value) - 0.5) * 1.999;
		}
	}
	return 0.0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
Engine::_MaximumRadius(double x0, double y0) const
{
	const double maximumX = std::max(x0, fWidth - x0);
	const double maximumY = std::max(y0, fHeight - y0);
	return std::sqrt(maximumX * maximumX + maximumY * maximumY);
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


int32_t
Engine::_RandomInt(int32_t upperBound)
{
	if (upperBound <= 1)
		return 0;
	std::uniform_int_distribution<int32_t> distribution(0, upperBound - 1);
	return distribution(fRandomEngine);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Engine::_StartPaletteTransition(int32_t paletteIndex)
{
	fSourceColours[paletteIndex] = fDestinationColours[paletteIndex];
	fDestinationColours[paletteIndex] = _NewDestinationColour();
	const Colour& source = fSourceColours[paletteIndex];
	const Colour& destination = fDestinationColours[paletteIndex];
	fColourSteps[paletteIndex] = std::max({
		std::abs(static_cast<int32_t>(destination.red) - source.red),
		std::abs(static_cast<int32_t>(destination.green) - source.green),
		std::abs(static_cast<int32_t>(destination.blue) - source.blue),
		1
	});
	fCurrentColourSteps[paletteIndex] = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
