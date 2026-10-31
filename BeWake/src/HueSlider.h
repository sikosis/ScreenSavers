/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef BE_WAKE_HUE_SLIDER_H
#define BE_WAKE_HUE_SLIDER_H

#include <Slider.h>


class HueSlider : public BSlider {
public:
	HueSlider(const char* name, const char* label, BMessage* message);

	void DrawBar() override;
};


#endif
