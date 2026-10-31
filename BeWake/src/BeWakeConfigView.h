/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef BE_WAKE_CONFIG_VIEW_H
#define BE_WAKE_CONFIG_VIEW_H

#include <View.h>


class BMenu;
class BMenuField;
class HueSlider;


class BeWakeConfigView : public BView {
public:
	BeWakeConfigView(BRect frame, float* hue, bool* cycleColours,
		bool* randomColour, bool* wantReset);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	void _UpdateControls();

	BMenu* fColourMenu;
	BMenuField* fColourField;
	HueSlider* fHueSlider;
	float* fHue;
	bool* fCycleColours;
	bool* fRandomColour;
	bool* fWantReset;
};


#endif
