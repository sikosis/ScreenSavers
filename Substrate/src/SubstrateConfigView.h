/*
 * Copyright (c) 2014 Josef Gajdusek
 * Distributed under the MIT License. See ../LICENSE.
 */

#ifndef SUBSTRATE_CONFIG_VIEW_H
#define SUBSTRATE_CONFIG_VIEW_H

#include <View.h>

class BCheckBox;
class BSlider;


class SubstrateConfigView : public BView {
public:
	SubstrateConfigView(BRect frame, int32* numberOfCracks, int32* duration,
		int32* grains, int32* circlePercentage, bool* dark, bool* wantRestart);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	BSlider* fCrackSlider;
	BSlider* fDurationSlider;
	BSlider* fGrainSlider;
	BSlider* fCircleSlider;
	BCheckBox* fDarkCheckBox;
	int32* fNumberOfCracks;
	int32* fDuration;
	int32* fGrains;
	int32* fCirclePercentage;
	bool* fDark;
	bool* fWantRestart;
};


#endif
