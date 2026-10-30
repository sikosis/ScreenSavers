/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#ifndef BRAIN_WASH_CONFIG_VIEW_H
#define BRAIN_WASH_CONFIG_VIEW_H

#include <View.h>


class BSlider;


class BrainWashConfigView : public BView {
public:
	BrainWashConfigView(BRect frame, int32* cycleSpeed, int32* drawingGap,
		int32* drawingSpeed);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	BSlider* fCycleSpeedSlider;
	BSlider* fDrawingGapSlider;
	BSlider* fDrawingSpeedSlider;
	int32* fCycleSpeed;
	int32* fDrawingGap;
	int32* fDrawingSpeed;
};


#endif
