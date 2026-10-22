#ifndef TTFX_CLOCK_H
#define TTFX_CLOCK_H

#include <cstdint>
#include <string>
#include <vector>


namespace ttfx {

struct ClockCell {
	int32_t column;
	int32_t row;
};


int32_t BuildClockCells(const std::string& clock,
	std::vector<ClockCell>& cells);

}


#endif
