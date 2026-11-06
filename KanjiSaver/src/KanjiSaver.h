#ifndef KANJI_SAVER_H
#define KANJI_SAVER_H

#include "KanjiSaverEngine.h"
#include "TTFXEngine.h"

#include <Font.h>
#include <ScreenSaver.h>
#include <String.h>

#include <atomic>
#include <vector>


class BBitmap;


class KanjiSaverScreenSaver : public BScreenSaver {
public:
	KanjiSaverScreenSaver(BMessage* archive, image_id image);
	~KanjiSaverScreenSaver() override;

	status_t InitCheck() override;
	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void StopSaver() override;
	void Draw(BView* view, int32 frame) override;

	int32 CountFonts() const;
	BString FontLabelAt(int32 index) const;
	int32 SelectedFont() const;
	void SelectFont(int32 index);
	int32 IntervalSeconds() const;
	void SetIntervalSeconds(int32 seconds);
	bool UseTtfxEffects() const;
	void SetUseTtfxEffects(bool enabled);

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
	void _BeginGlyph(const BFont& font, bigtime_t now);
	bool _EnsureBuffer(const BRect& bounds);
	void _UpdateTickSize();

	kanjisaver::Engine fEngine;
	ttfx::EffectEngine fEffectEngine;
	std::mt19937 fEffectRandom;
	std::vector<FontInfo> fFonts;
	std::string fGlyph;
	kanjisaver::Colour fColour;
	bigtime_t fGlyphStarted;
	bigtime_t fNextGlyphAt;
	bigtime_t fAppliedTickSize;
	std::atomic<int32> fFontIndex;
	std::atomic<int32> fIntervalSeconds;
	std::atomic<bool> fUseTtfxEffects;
	std::atomic<bool> fRefreshGlyph;
	std::atomic<bool> fRestartEffect;
	std::atomic<bool> fIntervalChanged;
	bool fHasGlyph;
	BBitmap* fOffscreenBitmap;
	BView* fOffscreenView;
	BBitmap* fGlyphBitmap;
	BView* fGlyphView;
};


#endif
