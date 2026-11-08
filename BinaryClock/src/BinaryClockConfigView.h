#ifndef BINARY_CLOCK_CONFIG_VIEW_H
#define BINARY_CLOCK_CONFIG_VIEW_H

#include <View.h>


class BCheckBox;
class BSlider;


class BinaryClockConfigView : public BView {
public:
	BinaryClockConfigView(BRect frame, int32* effectInterval,
		int32* transitionDuration, bool* showSeconds, bool* use24HourTime,
		bool* showBitWeights, bool* showGroupLabels, bool* useTtfxEffects,
		bool* randomiseEffects);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	void _UpdateIntervalLabel();
	void _UpdateTransitionLabel();

	BSlider* fEffectIntervalSlider;
	BSlider* fTransitionDurationSlider;
	BCheckBox* fShowSecondsCheckBox;
	BCheckBox* fUse24HourTimeCheckBox;
	BCheckBox* fShowBitWeightsCheckBox;
	BCheckBox* fShowGroupLabelsCheckBox;
	BCheckBox* fUseTtfxEffectsCheckBox;
	BCheckBox* fRandomiseEffectsCheckBox;
	int32* fEffectInterval;
	int32* fTransitionDuration;
	bool* fShowSeconds;
	bool* fUse24HourTime;
	bool* fShowBitWeights;
	bool* fShowGroupLabels;
	bool* fUseTtfxEffects;
	bool* fRandomiseEffects;
};


#endif
