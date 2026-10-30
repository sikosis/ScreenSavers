/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#ifndef BRAIN_WASH_ENGINE_H
#define BRAIN_WASH_ENGINE_H

#include <array>
#include <cstdint>
#include <random>
#include <vector>


namespace brainwash {

struct Colour {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	bool operator==(const Colour& other) const;
};


struct Point {
	double x;
	double y;
};


int32_t DrawingLinesPerFrame(int32_t drawingSpeed);
int64_t DrawingGapMicroseconds(int32_t drawingGap);


class Engine {
public:
	explicit Engine(uint32_t seed);

	void Resize(int32_t width, int32_t height);
	void Reset();
	void BeginPattern();
	bool RenderLines(int32_t maximumLines);
	void AdvancePalette(int32_t steps);

	int32_t Width() const;
	int32_t Height() const;
	bool PatternComplete() const;
	bool SecondPaletteActive() const;
	const std::vector<uint8_t>& Pixels() const;
	const std::array<Colour, 256>& Palette() const;

private:
	enum class PreKind {
		Cosine,
		Lens,
		Twist
	};

	enum class PostKind {
		SpiralCosine,
		SpiralCosineInverse,
		SpiralLinear,
		SpiralLinearInverse,
		WheelCosine,
		WheelLinear,
		Snail,
		Grid,
		Waves,
		Field,
		CircleCosine,
		CircleLinear,
		Hole
	};

	struct Transform {
		int32_t kind;
		double x0;
		double y0;
		double first;
		double second;
		double third;
	};

	void _BuildPalette();
	Colour _ColourBetween(const Colour& source, const Colour& destination,
		int32_t step, int32_t steps) const;
	void _CreateTransforms();
	Colour _NewDestinationColour();
	bool _NextLine(std::vector<Point>& points);
	Point _ApplyPreTransform(Point point, const Transform& transform) const;
	double _ApplyPostTransform(const Point& point,
		const Transform& transform) const;
	double _MaximumRadius(double x0, double y0) const;
	double _RandomDouble(double minimum, double maximum);
	int32_t _RandomInt(int32_t upperBound);
	void _StartPaletteTransition(int32_t paletteIndex);

	std::mt19937 fRandomEngine;
	std::vector<uint8_t> fPixels;
	std::array<Colour, 256> fPalette;
	std::array<Colour, 2> fSourceColours;
	std::array<Colour, 2> fDestinationColours;
	std::array<Colour, 2> fCurrentColours;
	std::array<int32_t, 2> fColourSteps;
	std::array<int32_t, 2> fCurrentColourSteps;
	std::vector<Transform> fPreTransforms;
	std::vector<Transform> fPostTransforms;
	int32_t fWidth;
	int32_t fHeight;
	int32_t fPaletteOffset;
	int32_t fCurrentLayer;
	int32_t fNextLayer;
	int32_t fLeft;
	int32_t fTop;
	int32_t fRight;
	int32_t fBottom;
	int32_t fSide;
	bool fPatternComplete;
	bool fSecondPaletteActive;
};

}


#endif
