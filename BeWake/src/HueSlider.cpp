/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#include "HueSlider.h"

#include "BeWakeEngine.h"

#include <View.h>

#include <algorithm>


HueSlider::HueSlider(const char* name, const char* label, BMessage* message)
	:
	BSlider(name, label, message, 0, 359, B_HORIZONTAL, B_TRIANGLE_THUMB)
{
	SetLimitLabels("0°", "359°");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
HueSlider::DrawBar()
{
	BView* view = OffscreenView();
	if (view == nullptr)
		return;

	const BRect bar = BarFrame();
	const double width = std::max<double>(bar.IntegerWidth(), 1.0);
	for (int32 x = static_cast<int32>(bar.left);
		x <= static_cast<int32>(bar.right); x++) {
		const bewake::Colour colour = bewake::HslToRgb((x - bar.left) / width,
			1.0, 0.5);
		view->SetHighColor(colour.red, colour.green, colour.blue);
		view->StrokeLine(BPoint(x, bar.top), BPoint(x, bar.bottom));
	}
	view->SetHighColor(118, 118, 118);
	view->StrokeRect(bar);
}
//---------------------------------------------------------------------------------------------------------------------------------//
