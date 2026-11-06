/*
 * KanjiSaver was created by Jens Kilian in 1999-2000.
 * See ../LICENSE for licensing information.
 */

#include "KanjiSaver.h"

#include "KanjiSaverConfigView.h"

#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <Point.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>


namespace {

const char* const kFamilyKey = "family";
const char* const kStyleKey = "style";
const char* const kIntervalKey = "interval_seconds";
const int32 kDefaultIntervalSeconds = 5;
const int32 kGlyphAttempts = 128;

}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* archive, image_id image)
{
	return new(std::nothrow) KanjiSaverScreenSaver(archive, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


KanjiSaverScreenSaver::KanjiSaverScreenSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fEngine(static_cast<uint32>(system_time())),
	fFontIndex(0),
	fIntervalSeconds(kDefaultIntervalSeconds)
{
	const char* savedFamily = "";
	const char* savedStyle = "";
	if (archive != nullptr) {
		const char* archivedFamily = nullptr;
		const char* archivedStyle = nullptr;
		if (archive->FindString(kFamilyKey, &archivedFamily) == B_OK
			&& archivedFamily != nullptr) {
			savedFamily = archivedFamily;
		}
		if (archive->FindString(kStyleKey, &archivedStyle) == B_OK
			&& archivedStyle != nullptr) {
			savedStyle = archivedStyle;
		}
		archive->FindInt32(kIntervalKey, &fIntervalSeconds);
	}
	fIntervalSeconds = kanjisaver::ClampIntervalSeconds(fIntervalSeconds);
	_LoadFonts(savedFamily, savedStyle);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
KanjiSaverScreenSaver::InitCheck()
{
	return fFonts.empty() ? B_NO_INIT : B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
KanjiSaverScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	const FontInfo* font = _SelectedFontInfo();
	if (font == nullptr)
		return B_NO_INIT;
	status_t status = into->AddString(kFamilyKey, font->family.String());
	if (status == B_OK)
		status = into->AddString(kStyleKey, font->style.String());
	if (status == B_OK)
		status = into->AddInt32(kIntervalKey, fIntervalSeconds);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;
	view->AddChild(new KanjiSaverConfigView(view->Bounds(), this));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
KanjiSaverScreenSaver::StartSaver(BView* view, bool)
{
	if (view == nullptr)
		return B_BAD_VALUE;
	const status_t status = InitCheck();
	if (status != B_OK)
		return status;

	view->SetViewColor(B_TRANSPARENT_COLOR);
	view->SetDrawingMode(B_OP_COPY);
	SetTickSize(static_cast<bigtime_t>(fIntervalSeconds) * 1000000);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	const BRect bounds = view->Bounds();
	view->SetLowColor(0, 0, 0, 255);
	view->FillRect(bounds, B_SOLID_LOW);

	const FontInfo* fontInfo = _SelectedFontInfo();
	if (fontInfo == nullptr)
		return;

	BFont font;
	font.SetFamilyAndStyle(fontInfo->identifier);
	font.SetSize(std::max(8.0f, bounds.Height() * 0.9f));

	kanjisaver::Colour colour = {};
	const std::string glyph = _ChooseSupportedGlyph(font, colour);
	if (glyph.empty())
		return;

	font_height metrics;
	font.GetHeight(&metrics);
	float glyphWidth = font.StringWidth(glyph.c_str());
	const float maximumWidth = std::max(1.0f, bounds.Width() * 0.9f);
	const float glyphHeight = metrics.ascent + metrics.descent;
	const float maximumHeight = std::max(1.0f, bounds.Height() * 0.9f);
	const float scale = std::min({1.0f,
		glyphWidth > 0.0f ? maximumWidth / glyphWidth : 1.0f,
		glyphHeight > 0.0f ? maximumHeight / glyphHeight : 1.0f});
	if (scale < 1.0f) {
		font.SetSize(font.Size() * scale);
		font.GetHeight(&metrics);
		glyphWidth = font.StringWidth(glyph.c_str());
	}

	view->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE);
	view->SetHighColor(colour.red, colour.green, colour.blue, colour.alpha);
	const float centreX = (bounds.left + bounds.right) / 2.0f;
	const float centreY = (bounds.top + bounds.bottom) / 2.0f;
	const BPoint origin(centreX - glyphWidth / 2.0f,
		centreY + (metrics.ascent - metrics.descent) / 2.0f);
	view->DrawString(glyph.c_str(), origin);
	view->Sync();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
KanjiSaverScreenSaver::CountFonts() const
{
	return static_cast<int32>(fFonts.size());
}
//---------------------------------------------------------------------------------------------------------------------------------//


BString
KanjiSaverScreenSaver::FontLabelAt(int32 index) const
{
	if (index < 0 || index >= CountFonts())
		return BString();
	BString label(fFonts[index].family);
	label << " " << fFonts[index].style;
	return label;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
KanjiSaverScreenSaver::SelectedFont() const
{
	return fFontIndex;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::SelectFont(int32 index)
{
	if (index >= 0 && index < CountFonts())
		fFontIndex = index;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
KanjiSaverScreenSaver::IntervalSeconds() const
{
	return fIntervalSeconds;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::SetIntervalSeconds(int32 seconds)
{
	fIntervalSeconds = kanjisaver::ClampIntervalSeconds(seconds);
	SetTickSize(static_cast<bigtime_t>(fIntervalSeconds) * 1000000);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::_LoadFonts(const char* savedFamily,
	const char* savedStyle)
{
	if (savedFamily == nullptr)
		savedFamily = "";
	if (savedStyle == nullptr)
		savedStyle = "";

	BFont font;
	const int32 familyCount = count_font_families();
	for (int32 familyIndex = 0; familyIndex < familyCount; familyIndex++) {
		font_family family;
		if (get_font_family(familyIndex, &family) != B_OK)
			continue;
		const int32 styleCount = count_font_styles(family);
		for (int32 styleIndex = 0; styleIndex < styleCount; styleIndex++) {
			font_style style;
			if (get_font_style(family, styleIndex, &style) != B_OK
				|| font.SetFamilyAndStyle(family, style) != B_OK
				|| !font.IncludesBlock(kanjisaver::kMinimumCodePoint,
					kanjisaver::kMaximumCodePoint)) {
				continue;
			}

			fFonts.push_back({family, style, font.FamilyAndStyle()});
			if (std::strcmp(family, savedFamily) == 0
				&& std::strcmp(style, savedStyle) == 0) {
				fFontIndex = CountFonts() - 1;
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


const KanjiSaverScreenSaver::FontInfo*
KanjiSaverScreenSaver::_SelectedFontInfo() const
{
	if (fFontIndex < 0 || fFontIndex >= CountFonts())
		return nullptr;
	return &fFonts[fFontIndex];
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
KanjiSaverScreenSaver::_ChooseSupportedGlyph(const BFont& font,
	kanjisaver::Colour& colour)
{
	std::string fallback;
	for (int32 attempt = 0; attempt < kGlyphAttempts; attempt++) {
		const kanjisaver::Frame frame = fEngine.NextFrame();
		const std::string glyph = kanjisaver::Utf8ForCodePoint(frame.codePoint);
		if (glyph.empty())
			continue;
		fallback = glyph;
		colour = frame.colour;
		bool hasGlyph = false;
		font.GetHasGlyphs(glyph.c_str(), 1, &hasGlyph, false);
		if (hasGlyph)
			return glyph;
	}
	return fallback;
}
//---------------------------------------------------------------------------------------------------------------------------------//
