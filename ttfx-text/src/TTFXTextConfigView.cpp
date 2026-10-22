#include "TTFXTextConfigView.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Slider.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>


namespace {

enum {
	kTextChanged = 'text',
	kEffectIntervalChanged = 'efin',
	kTransitionDurationChanged = 'trdu',
	kRandomiseEffectsChanged = 'rand',
	kShowEffectNameChanged = 'name'
};

}


TTFXTextConfigView::TTFXTextConfigView(BRect frame, BString* text,
	int32* effectInterval, int32* transitionDuration, bool* randomiseEffects,
	bool* showEffectName)
	:
	BView(frame, "ttfx-text settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fTextControl(new BTextControl("text", "Text", text->String(),
		new BMessage(kTextChanged))),
	fEffectIntervalSlider(new BSlider("effect interval", "Change effect every",
		new BMessage(kEffectIntervalChanged), 5, 60, B_HORIZONTAL, B_BLOCK_THUMB)),
	fTransitionDurationSlider(new BSlider("transition duration", "Effect duration",
		new BMessage(kTransitionDurationChanged), 1, 5, B_HORIZONTAL, B_BLOCK_THUMB)),
	fRandomiseEffectsCheckBox(new BCheckBox("randomise", "Randomise effect order",
		new BMessage(kRandomiseEffectsChanged))),
	fShowEffectNameCheckBox(new BCheckBox("show effect name", "Show effect name",
		new BMessage(kShowEffectNameChanged))),
	fText(text),
	fEffectInterval(effectInterval),
	fTransitionDuration(transitionDuration),
	fRandomiseEffects(randomiseEffects),
	fShowEffectName(showEffectName)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BStringView* title = new BStringView("title", "ttfx-text");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"Animate your own text with native ttfx effects");

	fTextControl->SetModificationMessage(new BMessage(kTextChanged));
	fTextControl->TextView()->SetMaxBytes(16);
	fEffectIntervalSlider->SetLimitLabels("5 seconds", "60 seconds");
	fEffectIntervalSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fEffectIntervalSlider->SetHashMarkCount(12);
	fEffectIntervalSlider->SetValue(*fEffectInterval);
	fTransitionDurationSlider->SetLimitLabels("Quick", "Slow");
	fTransitionDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fTransitionDurationSlider->SetHashMarkCount(5);
	fTransitionDurationSlider->SetValue(*fTransitionDuration);
	fRandomiseEffectsCheckBox->SetValue(*fRandomiseEffects
		? B_CONTROL_ON : B_CONTROL_OFF);
	fShowEffectNameCheckBox->SetValue(*fShowEffectName
		? B_CONTROL_ON : B_CONTROL_OFF);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fTextControl)
		.Add(fEffectIntervalSlider)
		.Add(fTransitionDurationSlider)
		.Add(fRandomiseEffectsCheckBox)
		.Add(fShowEffectNameCheckBox)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fTextControl->SetTarget(this);
	fEffectIntervalSlider->SetTarget(this);
	fTransitionDurationSlider->SetTarget(this);
	fRandomiseEffectsCheckBox->SetTarget(this);
	fShowEffectNameCheckBox->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kTextChanged:
			*fText = fTextControl->Text();
			break;
		case kEffectIntervalChanged:
			*fEffectInterval = fEffectIntervalSlider->Value();
			break;
		case kTransitionDurationChanged:
			*fTransitionDuration = fTransitionDurationSlider->Value();
			break;
		case kRandomiseEffectsChanged:
			*fRandomiseEffects = fRandomiseEffectsCheckBox->Value() == B_CONTROL_ON;
			break;
		case kShowEffectNameChanged:
			*fShowEffectName = fShowEffectNameCheckBox->Value() == B_CONTROL_ON;
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
