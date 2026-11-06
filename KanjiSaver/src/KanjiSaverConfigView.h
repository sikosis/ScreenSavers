#ifndef KANJI_SAVER_CONFIG_VIEW_H
#define KANJI_SAVER_CONFIG_VIEW_H

#include <View.h>


class BListView;
class BSlider;
class BCheckBox;
class KanjiSaverScreenSaver;


class KanjiSaverConfigView : public BView {
public:
	KanjiSaverConfigView(BRect frame, KanjiSaverScreenSaver* saver);
	~KanjiSaverConfigView() override;

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	KanjiSaverScreenSaver* fSaver;
	BListView* fFontList;
	BCheckBox* fTtfxCheckBox;
	BSlider* fIntervalSlider;
};


#endif
