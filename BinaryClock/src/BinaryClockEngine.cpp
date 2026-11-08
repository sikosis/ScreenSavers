#include "BinaryClockEngine.h"

#include <algorithm>


namespace binaryclock {

ClockDigits
DecodeClock(int hour, int minute, int second, bool showSeconds,
	bool use24HourTime)
{
	ClockDigits clock = {};
	const int hour24 = std::clamp(hour, 0, 23);
	int displayHour = hour24;
	if (!use24HourTime) {
		displayHour = hour24 % 12;
		if (displayHour == 0)
			displayHour = 12;
	}

	const int displayMinute = std::clamp(minute, 0, 59);
	const int displaySecond = std::clamp(second, 0, 59);

	clock.values[0] = displayHour / 10;
	clock.values[1] = displayHour % 10;
	clock.values[2] = displayMinute / 10;
	clock.values[3] = displayMinute % 10;
	clock.values[4] = displaySecond / 10;
	clock.values[5] = displaySecond % 10;
	clock.count = showSeconds ? 6 : 4;
	return clock;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
BuildBitCells(const ClockDigits& clock, std::vector<BitCell>& cells)
{
	cells.clear();
	const int columns = std::clamp(clock.count, 0, kMaximumDigits);
	for (int column = 0; column < columns; column++) {
		const int value = std::clamp(clock.values[column], 0, 9);
		for (int row = 0; row < kLedRows; row++) {
			const int weight = 1 << (kLedRows - 1 - row);
			cells.push_back({column, row, weight, (value & weight) != 0});
		}
	}
	return static_cast<int>(cells.size());
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
LitBitCount(const std::vector<BitCell>& cells)
{
	int count = 0;
	for (const BitCell& cell : cells) {
		if (cell.lit)
			count++;
	}
	return count;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
ColumnGroup(int column)
{
	return std::max(0, column) / 2;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
ColumnPosition(int column)
{
	const int nonNegative = std::max(0, column);
	return nonNegative + (nonNegative / 2) * kGroupGap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
