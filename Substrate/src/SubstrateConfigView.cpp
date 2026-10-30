/*
 * Copyright (c) 2014 Josef Gajdusek
 * Distributed under the MIT License. See ../LICENSE.
 */

#include "SubstrateConfigView.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Slider.h>
#include <StringView.h>


namespace {

enum {
	kCracksChanged = 'scrn',
	kDurationChanged = 'sdur',
	kGrainsChanged = 'sgrs',
	kCircleChanged = 'scrc',
	kDarkChanged = 'sdrk'
};

}


SubstrateConfigView::SubstrateConfigView(BRect frame, int32* numberOfCracks,
	int32* duration, int32* grains, int32* circlePercentage, bool* dark,
	bool* wantRestart)
	:
	BView(frame, "Substrate settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fCrackSlider(new BSlider("cracks", "Cracks", new BMessage(kCracksChanged),
		1, 20, B_HORIZONTAL, B_BLOCK_THUMB)),
	fDurationSlider(new BSlider("duration", "Duration",
		new BMessage(kDurationChanged), 3000, 10000, B_HORIZONTAL, B_BLOCK_THUMB)),
	fGrainSlider(new BSlider("grains", "Sand grains",
		new BMessage(kGrainsChanged), 0, 50, B_HORIZONTAL, B_BLOCK_THUMB)),
	fCircleSlider(new BSlider("circles", "Circular cracks",
		new BMessage(kCircleChanged), 0, 100, B_HORIZONTAL, B_BLOCK_THUMB)),
	fDarkCheckBox(new BCheckBox("dark", "Dark background", new BMessage(kDarkChanged))),
	fNumberOfCracks(numberOfCracks),
	fDuration(duration),
	fGrains(grains),
	fCirclePercentage(circlePercentage),
	fDark(dark),
	fWantRestart(wantRestart)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* title = new BStringView("title", "Substrate");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"Ported from XScreenSaver by Josef Gajdusek");

	fCrackSlider->SetLimitLabels("1", "20");
	fCrackSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fCrackSlider->SetHashMarkCount(20);
	fCrackSlider->SetValue(*fNumberOfCracks);

	fDurationSlider->SetLimitLabels("30 seconds", "100 seconds");
	fDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fDurationSlider->SetHashMarkCount(8);
	fDurationSlider->SetValue(*fDuration);

	fGrainSlider->SetLimitLabels("None", "Dense");
	fGrainSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fGrainSlider->SetHashMarkCount(11);
	fGrainSlider->SetValue(*fGrains);

	fCircleSlider->SetLimitLabels("Straight", "Circular");
	fCircleSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fCircleSlider->SetHashMarkCount(11);
	fCircleSlider->SetValue(*fCirclePercentage);
	fDarkCheckBox->SetValue(*fDark ? B_CONTROL_ON : B_CONTROL_OFF);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fCrackSlider)
		.Add(fDurationSlider)
		.Add(fGrainSlider)
		.Add(fCircleSlider)
		.Add(fDarkCheckBox)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fCrackSlider->SetTarget(this);
	fDurationSlider->SetTarget(this);
	fGrainSlider->SetTarget(this);
	fCircleSlider->SetTarget(this);
	fDarkCheckBox->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kCracksChanged:
			*fNumberOfCracks = fCrackSlider->Value();
			*fWantRestart = true;
			break;
		case kDurationChanged:
			*fDuration = fDurationSlider->Value();
			break;
		case kGrainsChanged:
			*fGrains = fGrainSlider->Value();
			break;
		case kCircleChanged:
			*fCirclePercentage = fCircleSlider->Value();
			*fWantRestart = true;
			break;
		case kDarkChanged:
			*fDark = fDarkCheckBox->Value() == B_CONTROL_ON;
			*fWantRestart = true;
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

