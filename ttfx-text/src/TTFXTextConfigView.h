#ifndef TTFX_TEXT_CONFIG_VIEW_H
#define TTFX_TEXT_CONFIG_VIEW_H

#include <View.h>


class BCheckBox;
class BSlider;
class BString;
class BTextControl;


class TTFXTextConfigView : public BView {
public:
	TTFXTextConfigView(BRect frame, BString* text, int32* effectInterval,
		int32* transitionDuration, bool* randomiseEffects);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	BTextControl* fTextControl;
	BSlider* fEffectIntervalSlider;
	BSlider* fTransitionDurationSlider;
	BCheckBox* fRandomiseEffectsCheckBox;
	BString* fText;
	int32* fEffectInterval;
	int32* fTransitionDuration;
	bool* fRandomiseEffects;
};


#endif
