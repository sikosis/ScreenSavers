/*
 * Copyright (c) 2014 Josef Gajdusek
 * Distributed under the MIT License. See ../LICENSE.
 */

#include "SubstrateMath.h"

#include <algorithm>
#include <cmath>


namespace substrate {

double
NormaliseAngle(double angle)
{
	angle = std::fmod(angle, 360.0);
	if (angle < 0.0)
		angle += 360.0;
	return angle;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
AngularDifference(double first, double second)
{
	double difference = std::abs(NormaliseAngle(first) - NormaliseAngle(second));
	return std::min(difference, 360.0 - difference);
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
SandPositionFactor(int32_t index, int32_t grainCount, double gain)
{
	if (grainCount <= 1 || index <= 0)
		return 0.0;

	gain = std::clamp(gain, 0.0, 1.0);
	const double position = static_cast<double>(index) * gain
		/ static_cast<double>(grainCount - 1);
	return std::sin(std::sin(position));
}
//---------------------------------------------------------------------------------------------------------------------------------//


int64_t
DurationMicroseconds(int32_t durationUnits)
{
	return static_cast<int64_t>(std::max(durationUnits, 1)) * 10000;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
