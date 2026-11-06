/*
 * KanjiSaver was created by Jens Kilian in 1999-2000.
 * See ../LICENSE for licensing information.
 */

#include "KanjiSaverConfigView.h"

#include "KanjiSaver.h"

#include <LayoutBuilder.h>
#include <ListItem.h>
#include <ListView.h>
#include <Message.h>
#include <ScrollView.h>
#include <Slider.h>
#include <StringView.h>


namespace {

enum {
	kFontSelected = 'ksfs',
	kIntervalChanged = 'ksic'
};

}


KanjiSaverConfigView::KanjiSaverConfigView(BRect frame,
	KanjiSaverScreenSaver* saver)
	:
	BView(frame, "KanjiSaver settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fSaver(saver),
	fFontList(new BListView("CJK fonts", B_SINGLE_SELECTION)),
	fIntervalSlider(new BSlider("interval", "Change character every",
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
		fIntervalSlider->SetValue(fSaver->IntervalSeconds());
	}
	fFontList->SetSelectionMessage(new BMessage(kFontSelected));
	fIntervalSlider->SetLimitLabels("1 second", "15 seconds");
	fIntervalSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fIntervalSlider->SetHashMarkCount(15);

	BScrollView* scrollView = new BScrollView("font scroll", fFontList,
		B_FOLLOW_ALL, 0, false, true);
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fontLabel)
		.Add(scrollView, 1.0f)
		.Add(fIntervalSlider)
		.AddGlue();
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
		case kIntervalChanged:
			fSaver->SetIntervalSeconds(fIntervalSlider->Value());
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
