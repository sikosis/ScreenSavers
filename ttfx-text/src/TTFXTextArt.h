#ifndef TTFX_TEXT_ART_H
#define TTFX_TEXT_ART_H

#include <cstdint>
#include <string>
#include <vector>


namespace ttfx {

struct TextCell {
	int32_t column;
	int32_t row;
};


int32_t BuildTextCells(const std::string& text, std::vector<TextCell>& cells);

}


#endif
