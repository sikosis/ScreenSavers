/*
 * Copyright (c) 2014 Josef Gajdusek
 * Distributed under the MIT License. See ../LICENSE.
 */

#ifndef SUBSTRATE_MATH_H
#define SUBSTRATE_MATH_H

#include <cstdint>


namespace substrate {

double NormaliseAngle(double angle);
double AngularDifference(double first, double second);
double SandPositionFactor(int32_t index, int32_t grainCount, double gain);
int64_t DurationMicroseconds(int32_t durationUnits);

}


#endif
