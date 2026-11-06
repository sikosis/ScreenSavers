#ifndef KANJI_SAVER_H
#define KANJI_SAVER_H

#include "KanjiSaverEngine.h"

#include <Font.h>
#include <ScreenSaver.h>
#include <String.h>

#include <vector>


class KanjiSaverScreenSaver : public BScreenSaver {
public:
	KanjiSaverScreenSaver(BMessage* archive, image_id image);

	status_t InitCheck() override;
	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void Draw(BView* view, int32 frame) override;

	int32 CountFonts() const;
	BString FontLabelAt(int32 index) const;
	int32 SelectedFont() const;
	void SelectFont(int32 index);
	int32 IntervalSeconds() const;
	void SetIntervalSeconds(int32 seconds);

private:
	struct FontInfo {
		BString family;
		BString style;
		uint32 identifier;
	};

	void _LoadFonts(const char* savedFamily, const char* savedStyle);
	const FontInfo* _SelectedFontInfo() const;
	std::string _ChooseSupportedGlyph(const BFont& font,
		kanjisaver::Colour& colour);

	kanjisaver::Engine fEngine;
	std::vector<FontInfo> fFonts;
	int32 fFontIndex;
	int32 fIntervalSeconds;
};


#endif
