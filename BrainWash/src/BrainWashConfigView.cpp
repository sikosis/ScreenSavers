/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#include "BrainWashConfigView.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <Slider.h>
#include <StringView.h>


namespace {

enum {
	kCycleSpeedChanged = 'bwcs',
	kDrawingGapChanged = 'bwdg',
	kDrawingSpeedChanged = 'bwds'
};

}


BrainWashConfigView::BrainWashConfigView(BRect frame, int32* cycleSpeed,
	int32* drawingGap, int32* drawingSpeed)
	:
	BView(frame, "BrainWash settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fCycleSpeedSlider(new BSlider("cycle speed", "Colour cycle speed",
		new BMessage(kCycleSpeedChanged), 1, 10, B_HORIZONTAL, B_BLOCK_THUMB)),
	fDrawingGapSlider(new BSlider("drawing gap", "Gap between patterns",
		new BMessage(kDrawingGapChanged), 0, 10, B_HORIZONTAL, B_BLOCK_THUMB)),
	fDrawingSpeedSlider(new BSlider("drawing speed", "Drawing speed",
		new BMessage(kDrawingSpeedChanged), 1, 9, B_HORIZONTAL, B_BLOCK_THUMB)),
	fCycleSpeed(cycleSpeed),
	fDrawingGap(drawingGap),
	fDrawingSpeed(drawingSpeed)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* title = new BStringView("title", "BrainWash");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"Generative colour fields by Stephan Aßmus and Ingo Weinhold");

	fCycleSpeedSlider->SetLimitLabels("Slow", "Fast");
	fCycleSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fCycleSpeedSlider->SetHashMarkCount(10);
	fCycleSpeedSlider->SetValue(*fCycleSpeed);

	fDrawingGapSlider->SetLimitLabels("None", "10 seconds");
	fDrawingGapSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fDrawingGapSlider->SetHashMarkCount(11);
	fDrawingGapSlider->SetValue(*fDrawingGap);

	fDrawingSpeedSlider->SetLimitLabels("Slow", "Fast");
	fDrawingSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fDrawingSpeedSlider->SetHashMarkCount(9);
	fDrawingSpeedSlider->SetValue(*fDrawingSpeed);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fCycleSpeedSlider)
		.Add(fDrawingGapSlider)
		.Add(fDrawingSpeedSlider)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fCycleSpeedSlider->SetTarget(this);
	fDrawingGapSlider->SetTarget(this);
	fDrawingSpeedSlider->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kCycleSpeedChanged:
			*fCycleSpeed = fCycleSpeedSlider->Value();
			break;
		case kDrawingGapChanged:
			*fDrawingGap = fDrawingGapSlider->Value();
			break;
		case kDrawingSpeedChanged:
			*fDrawingSpeed = fDrawingSpeedSlider->Value();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
