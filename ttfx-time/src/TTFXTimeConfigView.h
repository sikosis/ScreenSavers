#ifndef TTFX_TIME_CONFIG_VIEW_H
#define TTFX_TIME_CONFIG_VIEW_H

#include <View.h>


class BCheckBox;
class BSlider;


class TTFXTimeConfigView : public BView {
public:
	TTFXTimeConfigView(BRect frame, int32* effectInterval,
		int32* transitionDuration, bool* showSeconds, bool* use24HourTime,
		bool* randomiseEffects, bool* showEffectName);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	BSlider* fEffectIntervalSlider;
	BSlider* fTransitionDurationSlider;
	BCheckBox* fShowSecondsCheckBox;
	BCheckBox* fUse24HourTimeCheckBox;
	BCheckBox* fRandomiseEffectsCheckBox;
	BCheckBox* fShowEffectNameCheckBox;
	int32* fEffectInterval;
	int32* fTransitionDuration;
	bool* fShowSeconds;
	bool* fUse24HourTime;
	bool* fRandomiseEffects;
	bool* fShowEffectName;
};


#endif
