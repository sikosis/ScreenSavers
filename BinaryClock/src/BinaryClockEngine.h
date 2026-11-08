#ifndef BINARY_CLOCK_ENGINE_H
#define BINARY_CLOCK_ENGINE_H

#include <vector>


namespace binaryclock {

constexpr int kLedRows = 4;
constexpr int kMaximumDigits = 6;
constexpr double kGroupGap = 0.9;


struct ClockDigits {
	int values[kMaximumDigits];
	int count;
};


struct BitCell {
	int column;
	int row;
	int weight;
	bool lit;
};


ClockDigits DecodeClock(int hour, int minute, int second,
	bool showSeconds, bool use24HourTime);
int BuildBitCells(const ClockDigits& clock, std::vector<BitCell>& cells);
int LitBitCount(const std::vector<BitCell>& cells);
int ColumnGroup(int column);
double ColumnPosition(int column);

}


#endif
