/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#include "BeWakeConfigView.h"

#include "HueSlider.h"

#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <StringView.h>


namespace {

enum {
	kCycleColours = 'bwcc',
	kRandomColour = 'bwrc',
	kFixedColour = 'bwfc',
	kHueChanged = 'bwhu'
};

}


BeWakeConfigView::BeWakeConfigView(BRect frame, float* hue, bool* cycleColours,
	bool* randomColour, bool* wantReset)
	:
	BView(frame, "BeWake settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fColourMenu(new BMenu("colour mode")),
	fColourField(nullptr),
	fHueSlider(new HueSlider("hue", "Custom hue", new BMessage(kHueChanged))),
	fHue(hue),
	fCycleColours(cycleColours),
	fRandomColour(randomColour),
	fWantReset(wantReset)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* title = new BStringView("title", "BeWake");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"A colourful moving line wake by Mattia Tristo and Gabriele Biffi");

	fColourMenu->AddItem(new BMenuItem("Cycle colours",
		new BMessage(kCycleColours)));
	fColourMenu->AddItem(new BMenuItem("Random colour",
		new BMessage(kRandomColour)));
	fColourMenu->AddItem(new BMenuItem("Custom colour",
		new BMessage(kFixedColour)));
	fColourMenu->SetRadioMode(true);
	fColourMenu->SetLabelFromMarked(true);
	fColourField = new BMenuField("colour mode", "Colour mode", fColourMenu);

	fHueSlider->SetValue(static_cast<int32>(*fHue * 360.0f));
	fHueSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fHueSlider->SetHashMarkCount(13);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fColourField)
		.Add(fHueSlider)
		.AddGlue();
	_UpdateControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fColourMenu->SetTargetForItems(this);
	fHueSlider->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kCycleColours:
			*fCycleColours = true;
			*fRandomColour = false;
			*fWantReset = true;
			break;
		case kRandomColour:
			*fCycleColours = false;
			*fRandomColour = true;
			*fWantReset = true;
			break;
		case kFixedColour:
			*fCycleColours = false;
			*fRandomColour = false;
			*fWantReset = true;
			break;
		case kHueChanged:
			*fHue = fHueSlider->Value() / 360.0f;
			break;
		default:
			BView::MessageReceived(message);
			return;
	}
	_UpdateControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeConfigView::_UpdateControls()
{
	const int32 markedIndex = *fCycleColours ? 0 : (*fRandomColour ? 1 : 2);
	BMenuItem* item = fColourMenu->ItemAt(markedIndex);
	if (item != nullptr)
		item->SetMarked(true);
	fHueSlider->SetEnabled(markedIndex == 2);
}
//---------------------------------------------------------------------------------------------------------------------------------//
