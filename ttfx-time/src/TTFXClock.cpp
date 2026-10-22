#include "TTFXClock.h"


namespace ttfx {

namespace {

constexpr int32_t kClockRows = 7;

const char* const kDigitPatterns[][kClockRows] = {
	{"11111", "10001", "10001", "10001", "10001", "10001", "11111"},
	{"00100", "01100", "00100", "00100", "00100", "00100", "01110"},
	{"11111", "00001", "00001", "11111", "10000", "10000", "11111"},
	{"11111", "00001", "00001", "01111", "00001", "00001", "11111"},
	{"10001", "10001", "10001", "11111", "00001", "00001", "00001"},
	{"11111", "10000", "10000", "11111", "00001", "00001", "11111"},
	{"11111", "10000", "10000", "11111", "10001", "10001", "11111"},
	{"11111", "00001", "00010", "00100", "01000", "01000", "01000"},
	{"11111", "10001", "10001", "11111", "10001", "10001", "11111"},
	{"11111", "10001", "10001", "11111", "00001", "00001", "11111"}
};

const char* const kColonPattern[kClockRows]
	= {"0", "1", "1", "0", "1", "1", "0"};


}


int32_t
BuildClockCells(const std::string& clock, std::vector<ClockCell>& cells)
{
	cells.clear();
	int32_t totalColumns = 0;
	for (std::size_t index = 0; index < clock.size(); index++) {
		const char character = clock[index];
		const bool isColon = character == ':';
		const int32_t glyphWidth = isColon ? 1 : 5;
		if (isColon || (character >= '0' && character <= '9')) {
			for (int32_t column = 0; column < glyphWidth; column++) {
				for (int32_t row = 0; row < kClockRows; row++) {
					const char* pattern = isColon
						? kColonPattern[row] : kDigitPatterns[character - '0'][row];
					if (pattern[column] == '1')
						cells.push_back({totalColumns + column, row});
				}
			}
		}
		totalColumns += glyphWidth;
		if (index + 1 < clock.size())
			totalColumns++;
	}
	return totalColumns;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}
