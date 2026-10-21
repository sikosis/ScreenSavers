#include "TTFXTimeConfigView.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Slider.h>
#include <StringView.h>


namespace {

enum {
	kEffectIntervalChanged = 'efin',
	kTransitionDurationChanged = 'trdu',
	kShowSecondsChanged = 'secs',
	kUse24HourTimeChanged = '24hr',
	kRandomiseEffectsChanged = 'rand'
};

}


TTFXTimeConfigView::TTFXTimeConfigView(BRect frame, int32* effectInterval,
	int32* transitionDuration, bool* showSeconds, bool* use24HourTime,
	bool* randomiseEffects)
	:
	BView(frame, "ttfx-time settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fEffectIntervalSlider(new BSlider("effect interval", "Change effect every",
		new BMessage(kEffectIntervalChanged), 5, 60, B_HORIZONTAL, B_BLOCK_THUMB)),
	fTransitionDurationSlider(new BSlider("transition duration", "Effect duration",
		new BMessage(kTransitionDurationChanged), 1, 5, B_HORIZONTAL, B_BLOCK_THUMB)),
	fShowSecondsCheckBox(new BCheckBox("show seconds", "Show seconds",
		new BMessage(kShowSecondsChanged))),
	fUse24HourTimeCheckBox(new BCheckBox("24 hour time", "Use 24-hour time",
		new BMessage(kUse24HourTimeChanged))),
	fRandomiseEffectsCheckBox(new BCheckBox("randomise", "Randomise effect order",
		new BMessage(kRandomiseEffectsChanged))),
	fEffectInterval(effectInterval),
	fTransitionDuration(transitionDuration),
	fShowSeconds(showSeconds),
	fUse24HourTime(use24HourTime),
	fRandomiseEffects(randomiseEffects)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BStringView* title = new BStringView("title", "ttfx-time");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"Current time animated with native ttfx effects");

	fEffectIntervalSlider->SetLimitLabels("5 seconds", "60 seconds");
	fEffectIntervalSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fEffectIntervalSlider->SetHashMarkCount(12);
	fEffectIntervalSlider->SetValue(*fEffectInterval);
	fTransitionDurationSlider->SetLimitLabels("Quick", "Slow");
	fTransitionDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fTransitionDurationSlider->SetHashMarkCount(5);
	fTransitionDurationSlider->SetValue(*fTransitionDuration);
	fShowSecondsCheckBox->SetValue(*fShowSeconds ? B_CONTROL_ON : B_CONTROL_OFF);
	fUse24HourTimeCheckBox->SetValue(*fUse24HourTime ? B_CONTROL_ON : B_CONTROL_OFF);
	fRandomiseEffectsCheckBox->SetValue(*fRandomiseEffects
		? B_CONTROL_ON : B_CONTROL_OFF);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fEffectIntervalSlider)
		.Add(fTransitionDurationSlider)
		.Add(fShowSecondsCheckBox)
		.Add(fUse24HourTimeCheckBox)
		.Add(fRandomiseEffectsCheckBox)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fEffectIntervalSlider->SetTarget(this);
	fTransitionDurationSlider->SetTarget(this);
	fShowSecondsCheckBox->SetTarget(this);
	fUse24HourTimeCheckBox->SetTarget(this);
	fRandomiseEffectsCheckBox->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kEffectIntervalChanged:
			*fEffectInterval = fEffectIntervalSlider->Value();
			break;
		case kTransitionDurationChanged:
			*fTransitionDuration = fTransitionDurationSlider->Value();
			break;
		case kShowSecondsChanged:
			*fShowSeconds = fShowSecondsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kUse24HourTimeChanged:
			*fUse24HourTime = fUse24HourTimeCheckBox->Value() == B_CONTROL_ON;
			break;
		case kRandomiseEffectsChanged:
			*fRandomiseEffects = fRandomiseEffectsCheckBox->Value() == B_CONTROL_ON;
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
