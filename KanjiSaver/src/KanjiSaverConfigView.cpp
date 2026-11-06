/*
 * KanjiSaver was created by Jens Kilian in 1999-2000.
 * See ../LICENSE for licensing information.
 */

#include "KanjiSaverConfigView.h"

#include "KanjiSaver.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <ListItem.h>
#include <ListView.h>
#include <Message.h>
#include <ScrollView.h>
#include <Size.h>
#include <Slider.h>
#include <String.h>
#include <StringView.h>


namespace {

enum {
	kFontSelected = 'ksfs',
	kTtfxChanged = 'kstf',
	kIntervalChanged = 'ksic'
};

}


KanjiSaverConfigView::KanjiSaverConfigView(BRect frame,
	KanjiSaverScreenSaver* saver)
	:
	BView(frame, "KanjiSaver settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fSaver(saver),
	fFontList(new BListView("CJK fonts", B_SINGLE_SELECTION_LIST)),
	fTtfxCheckBox(new BCheckBox("ttfx effects",
		"Assemble kanji with ttfx effects",
		new BMessage(kTtfxChanged))),
	fIntervalSlider(new BSlider("interval", "Character interval",
		new BMessage(kIntervalChanged), 1, 15, B_HORIZONTAL, B_BLOCK_THUMB))
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* title = new BStringView("title", "KanjiSaver");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"Random Japanese kanji by Jens Kilian");
	BStringView* fontLabel = new BStringView("font label", "CJK font");

	if (fSaver != nullptr) {
		for (int32 index = 0; index < fSaver->CountFonts(); index++) {
			const BString label = fSaver->FontLabelAt(index);
			fFontList->AddItem(new BStringItem(label.String()));
		}
		if (fSaver->CountFonts() > 0)
			fFontList->Select(fSaver->SelectedFont());
		else
			fontLabel->SetText("No installed font provides CJK ideographs");
		fTtfxCheckBox->SetValue(fSaver->UseTtfxEffects()
			? B_CONTROL_ON : B_CONTROL_OFF);
		fIntervalSlider->SetValue(fSaver->IntervalSeconds());
	}
	fFontList->SetSelectionMessage(new BMessage(kFontSelected));
	fIntervalSlider->SetLimitLabels("1 second", "15 seconds");
	fIntervalSlider->SetExplicitMinSize(BSize(160.0f, 52.0f));
	_UpdateIntervalLabel();

	BScrollView* scrollView = new BScrollView("font scroll", fFontList,
		B_FOLLOW_ALL, 0, false, true);
	scrollView->SetExplicitMinSize(BSize(160.0f, 64.0f));
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fTtfxCheckBox)
		.Add(fIntervalSlider)
		.Add(fontLabel)
		.Add(scrollView, 1.0f);
}
//---------------------------------------------------------------------------------------------------------------------------------//


KanjiSaverConfigView::~KanjiSaverConfigView()
{
	while (!fFontList->IsEmpty())
		delete fFontList->RemoveItem(fFontList->CountItems() - 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fFontList->SetTarget(this);
	fTtfxCheckBox->SetTarget(this);
	fIntervalSlider->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverConfigView::MessageReceived(BMessage* message)
{
	if (message == nullptr)
		return;
	if (fSaver == nullptr) {
		BView::MessageReceived(message);
		return;
	}

	switch (message->what) {
		case kFontSelected:
			fSaver->SelectFont(fFontList->CurrentSelection());
			break;
		case kTtfxChanged:
			fSaver->SetUseTtfxEffects(
				fTtfxCheckBox->Value() == B_CONTROL_ON);
			break;
		case kIntervalChanged:
			fSaver->SetIntervalSeconds(fIntervalSlider->Value());
			_UpdateIntervalLabel();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverConfigView::_UpdateIntervalLabel()
{
	const int32 seconds = fIntervalSlider->Value();
	BString label("Change character every: ");
	label << seconds << (seconds == 1 ? " second" : " seconds");
	fIntervalSlider->SetLabel(label.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//
