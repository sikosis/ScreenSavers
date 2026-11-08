#include "BinaryClockConfigView.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Size.h>
#include <Slider.h>
#include <String.h>
#include <StringView.h>


namespace {

enum {
	kEffectIntervalChanged = 'bcei',
	kTransitionDurationChanged = 'bctd',
	kShowSecondsChanged = 'bcss',
	kUse24HourTimeChanged = 'bc24',
	kShowBitWeightsChanged = 'bcbw',
	kShowGroupLabelsChanged = 'bcgl',
	kUseTtfxEffectsChanged = 'bctf',
	kRandomiseEffectsChanged = 'bcrd'
};

}


BinaryClockConfigView::BinaryClockConfigView(BRect frame, int32* effectInterval,
	int32* transitionDuration, bool* showSeconds, bool* use24HourTime,
	bool* showBitWeights, bool* showGroupLabels, bool* useTtfxEffects,
	bool* randomiseEffects)
	:
	BView(frame, "BinaryClock settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fEffectIntervalSlider(new BSlider("effect interval",
		"Change effect every", new BMessage(kEffectIntervalChanged),
		5, 60, B_HORIZONTAL, B_BLOCK_THUMB)),
	fTransitionDurationSlider(new BSlider("transition duration",
		"Transition duration", new BMessage(kTransitionDurationChanged),
		1, 5, B_HORIZONTAL, B_BLOCK_THUMB)),
	fShowSecondsCheckBox(new BCheckBox("show seconds", "Show seconds",
		new BMessage(kShowSecondsChanged))),
	fUse24HourTimeCheckBox(new BCheckBox("24 hour time", "Use 24-hour time",
		new BMessage(kUse24HourTimeChanged))),
	fShowBitWeightsCheckBox(new BCheckBox("bit weights", "Show bit weights",
		new BMessage(kShowBitWeightsChanged))),
	fShowGroupLabelsCheckBox(new BCheckBox("group labels", "Show H/M/S labels",
		new BMessage(kShowGroupLabelsChanged))),
	fUseTtfxEffectsCheckBox(new BCheckBox("ttfx effects",
		"Animate with ttfx effects", new BMessage(kUseTtfxEffectsChanged))),
	fRandomiseEffectsCheckBox(new BCheckBox("randomise",
		"Randomise effect order", new BMessage(kRandomiseEffectsChanged))),
	fEffectInterval(effectInterval),
	fTransitionDuration(transitionDuration),
	fShowSeconds(showSeconds),
	fUse24HourTime(use24HourTime),
	fShowBitWeights(showBitWeights),
	fShowGroupLabels(showGroupLabels),
	fUseTtfxEffects(useTtfxEffects),
	fRandomiseEffects(randomiseEffects)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetHighUIColor(B_PANEL_TEXT_COLOR);
	BStringView* title = new BStringView("title", "Binary Clock");
	title->SetFont(be_bold_font);
	title->SetHighUIColor(B_PANEL_TEXT_COLOR);
	BStringView* description = new BStringView("description",
		"Binary-coded decimal clock with optional ttfx effects");
	description->SetFont(be_plain_font);
	description->SetHighUIColor(B_PANEL_TEXT_COLOR);

	fEffectIntervalSlider->SetLimitLabels("5 seconds", "60 seconds");
	fEffectIntervalSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fEffectIntervalSlider->SetHashMarkCount(12);
	fEffectIntervalSlider->SetExplicitMinSize(BSize(180.0f, 52.0f));
	fEffectIntervalSlider->SetValue(*fEffectInterval);
	fTransitionDurationSlider->SetLimitLabels("Quick", "Slow");
	fTransitionDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fTransitionDurationSlider->SetHashMarkCount(5);
	fTransitionDurationSlider->SetExplicitMinSize(BSize(180.0f, 52.0f));
	fTransitionDurationSlider->SetValue(*fTransitionDuration);
	fShowSecondsCheckBox->SetValue(*fShowSeconds
		? B_CONTROL_ON : B_CONTROL_OFF);
	fUse24HourTimeCheckBox->SetValue(*fUse24HourTime
		? B_CONTROL_ON : B_CONTROL_OFF);
	fShowBitWeightsCheckBox->SetValue(*fShowBitWeights
		? B_CONTROL_ON : B_CONTROL_OFF);
	fShowGroupLabelsCheckBox->SetValue(*fShowGroupLabels
		? B_CONTROL_ON : B_CONTROL_OFF);
	fUseTtfxEffectsCheckBox->SetValue(*fUseTtfxEffects
		? B_CONTROL_ON : B_CONTROL_OFF);
	fRandomiseEffectsCheckBox->SetValue(*fRandomiseEffects
		? B_CONTROL_ON : B_CONTROL_OFF);
	_UpdateIntervalLabel();
	_UpdateTransitionLabel();

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fShowSecondsCheckBox)
			.Add(fUse24HourTimeCheckBox)
			.AddGlue()
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fShowBitWeightsCheckBox)
			.Add(fShowGroupLabelsCheckBox)
			.AddGlue()
		.End()
		.Add(fUseTtfxEffectsCheckBox)
		.Add(fEffectIntervalSlider)
		.Add(fTransitionDurationSlider)
		.Add(fRandomiseEffectsCheckBox)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fEffectIntervalSlider->SetTarget(this);
	fTransitionDurationSlider->SetTarget(this);
	fShowSecondsCheckBox->SetTarget(this);
	fUse24HourTimeCheckBox->SetTarget(this);
	fShowBitWeightsCheckBox->SetTarget(this);
	fShowGroupLabelsCheckBox->SetTarget(this);
	fUseTtfxEffectsCheckBox->SetTarget(this);
	fRandomiseEffectsCheckBox->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kEffectIntervalChanged:
			*fEffectInterval = fEffectIntervalSlider->Value();
			_UpdateIntervalLabel();
			break;
		case kTransitionDurationChanged:
			*fTransitionDuration = fTransitionDurationSlider->Value();
			_UpdateTransitionLabel();
			break;
		case kShowSecondsChanged:
			*fShowSeconds = fShowSecondsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kUse24HourTimeChanged:
			*fUse24HourTime = fUse24HourTimeCheckBox->Value() == B_CONTROL_ON;
			break;
		case kShowBitWeightsChanged:
			*fShowBitWeights = fShowBitWeightsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kShowGroupLabelsChanged:
			*fShowGroupLabels
				= fShowGroupLabelsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kUseTtfxEffectsChanged:
			*fUseTtfxEffects = fUseTtfxEffectsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kRandomiseEffectsChanged:
			*fRandomiseEffects
				= fRandomiseEffectsCheckBox->Value() == B_CONTROL_ON;
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockConfigView::_UpdateIntervalLabel()
{
	const int32 seconds = fEffectIntervalSlider->Value();
	BString label("Change effect every: ");
	label << seconds << (seconds == 1 ? " second" : " seconds");
	fEffectIntervalSlider->SetLabel(label.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockConfigView::_UpdateTransitionLabel()
{
	const int32 seconds = fTransitionDurationSlider->Value();
	BString label("Transition duration: ");
	label << seconds << (seconds == 1 ? " second" : " seconds");
	fTransitionDurationSlider->SetLabel(label.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//
