#include "../src/SubstrateMath.h"

#include <cassert>
#include <cmath>
#include <iostream>


namespace {

bool
nearly_equal(double first, double second)
{
	return std::abs(first - second) < 0.000001;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_angle_normalisation()
{
	assert(nearly_equal(substrate::NormaliseAngle(0.0), 0.0));
	assert(nearly_equal(substrate::NormaliseAngle(360.0), 0.0));
	assert(nearly_equal(substrate::NormaliseAngle(-90.0), 270.0));
	assert(nearly_equal(substrate::NormaliseAngle(810.0), 90.0));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_wrapped_angle_difference()
{
	assert(nearly_equal(substrate::AngularDifference(359.0, 1.0), 2.0));
	assert(nearly_equal(substrate::AngularDifference(10.0, 190.0), 180.0));
	assert(nearly_equal(substrate::AngularDifference(-10.0, 350.0), 0.0));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_sand_factor_handles_boundary_counts()
{
	assert(nearly_equal(substrate::SandPositionFactor(0, 0, 0.5), 0.0));
	assert(nearly_equal(substrate::SandPositionFactor(0, 1, 0.5), 0.0));
	assert(nearly_equal(substrate::SandPositionFactor(1, 1, 0.5), 0.0));
	assert(nearly_equal(substrate::SandPositionFactor(1, 2, -1.0), 0.0));
	assert(std::isfinite(substrate::SandPositionFactor(1, 2, 0.5)));
	assert(nearly_equal(substrate::SandPositionFactor(1, 2, 2.0),
		std::sin(std::sin(1.0))));
	assert(substrate::SandPositionFactor(49, 50, 1.0) > 0.0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
test_duration_uses_real_time()
{
	assert(substrate::DurationMicroseconds(3000) == 30000000);
	assert(substrate::DurationMicroseconds(10000) == 100000000);
	assert(substrate::DurationMicroseconds(0) == 10000);
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


int
main()
{
	test_angle_normalisation();
	test_wrapped_angle_difference();
	test_sand_factor_handles_boundary_counts();
	test_duration_uses_real_time();
	std::cout << "Substrate tests passed\n";
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//

