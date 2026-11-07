/*
 * KanjiSaver was created by Jens Kilian in 1999-2000.
 * See ../LICENSE for licensing information.
 */

#include "KanjiSaver.h"

#include "KanjiSaverConfigView.h"

#include <Bitmap.h>
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
const char* const kUseTtfxEffectsKey = "use_ttfx_effects";
const int32 kDefaultIntervalSeconds = 5;
const int32 kGlyphAttempts = 128;
const int32 kBlockColumns = 16;
const int32 kBlockRows = 16;
const bigtime_t kTtfxFrameInterval = 33333;


bool
bitmap_block_has_content(BBitmap* bitmap, const BRect& block)
{
	if (bitmap == nullptr || bitmap->Bits() == nullptr)
		return false;
	const BRect bounds = bitmap->Bounds();
	const int32 left = std::clamp(static_cast<int32>(std::floor(block.left)),
		0, static_cast<int32>(bounds.IntegerWidth()));
	const int32 right = std::clamp(static_cast<int32>(std::ceil(block.right)),
		0, static_cast<int32>(bounds.IntegerWidth()));
	const int32 top = std::clamp(static_cast<int32>(std::floor(block.top)),
		0, static_cast<int32>(bounds.IntegerHeight()));
	const int32 bottom = std::clamp(static_cast<int32>(std::ceil(block.bottom)),
		0, static_cast<int32>(bounds.IntegerHeight()));
	const uint8* bits = static_cast<const uint8*>(bitmap->Bits());
	const int32 bytesPerRow = bitmap->BytesPerRow();
	for (int32 y = top; y <= bottom; y++) {
		for (int32 x = left; x <= right; x++) {
			const uint8* pixel = bits + y * bytesPerRow + x * 4;
			if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0
				|| pixel[3] != 0) {
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


ttfx::Effect
next_assembly_effect(ttfx::Effect current)
{
	static const ttfx::Effect effects[] = {
		ttfx::Effect::kExpand,
		ttfx::Effect::kSlide,
		ttfx::Effect::kRain,
		ttfx::Effect::kScattered,
		ttfx::Effect::kWaves,
		ttfx::Effect::kMatrix
	};
	constexpr std::size_t count = sizeof(effects) / sizeof(effects[0]);
	for (std::size_t index = 0; index < count; index++) {
		if (effects[index] == current)
			return effects[(index + 1) % count];
	}
	return effects[0];
}
//---------------------------------------------------------------------------------------------------------------------------------//


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
	fEffectRandom(static_cast<uint32>(system_time())),
	fGlyph({}),
	fColour({255, 255, 255, 255}),
	fGlyphStarted(0),
	fNextGlyphAt(0),
	fAppliedTickSize(0),
	fFontIndex(0),
	fIntervalSeconds(kDefaultIntervalSeconds),
	fUseTtfxEffects(false),
	fRefreshGlyph(false),
	fRestartEffect(false),
	fIntervalChanged(false),
	fHasGlyph(false),
	fOffscreenBitmap(nullptr),
	fOffscreenView(nullptr),
	fGlyphBitmap(nullptr),
	fGlyphView(nullptr)
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
		int32 archivedInterval = kDefaultIntervalSeconds;
		bool archivedEffects = false;
		if (archive->FindInt32(kIntervalKey, &archivedInterval) == B_OK)
			fIntervalSeconds.store(archivedInterval);
		if (archive->FindBool(kUseTtfxEffectsKey, &archivedEffects) == B_OK)
			fUseTtfxEffects.store(archivedEffects);
	}
	fIntervalSeconds.store(kanjisaver::ClampIntervalSeconds(
		fIntervalSeconds.load()));
	_LoadFonts(savedFamily, savedStyle);
}
//---------------------------------------------------------------------------------------------------------------------------------//


KanjiSaverScreenSaver::~KanjiSaverScreenSaver()
{
	delete fOffscreenBitmap;
	delete fGlyphBitmap;
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
		status = into->AddInt32(kIntervalKey, fIntervalSeconds.load());
	if (status == B_OK)
		status = into->AddBool(kUseTtfxEffectsKey, fUseTtfxEffects.load());
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::StopSaver()
{
	delete fOffscreenBitmap;
	fOffscreenBitmap = nullptr;
	fOffscreenView = nullptr;
	delete fGlyphBitmap;
	fGlyphBitmap = nullptr;
	fGlyphView = nullptr;
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
	fGlyph.fill('\0');
	fGlyphStarted = 0;
	fNextGlyphAt = 0;
	fAppliedTickSize = 0;
	fRefreshGlyph.store(false);
	fRestartEffect.store(false);
	fIntervalChanged.store(false);
	fHasGlyph = false;
	fEffectEngine.SetEffect(ttfx::Effect::kExpand, fEffectRandom());
	_UpdateTickSize();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;
	_UpdateTickSize();
	const bigtime_t now = system_time();
	if (fRestartEffect.exchange(false))
		fGlyphStarted = now;
	if (fIntervalChanged.exchange(false)) {
		fNextGlyphAt = now
			+ static_cast<bigtime_t>(fIntervalSeconds.load()) * 1000000;
	}

	const BRect targetBounds = view->Bounds();
	if (!_EnsureBuffer(targetBounds) || !fOffscreenBitmap->Lock())
		return;
	const BRect bounds = fOffscreenBitmap->Bounds();
	BView* canvas = fOffscreenView;
	canvas->ConstrainClippingRegion(nullptr);
	canvas->SetDrawingMode(B_OP_COPY);
	canvas->SetLowColor(0, 0, 0, 255);
	canvas->FillRect(bounds, B_SOLID_LOW);

	const FontInfo* fontInfo = _SelectedFontInfo();
	if (fontInfo == nullptr) {
		canvas->Sync();
		fOffscreenBitmap->Unlock();
		view->SetDrawingMode(B_OP_COPY);
		view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
		return;
	}

	BFont font;
	font.SetFamilyAndStyle(fontInfo->identifier);
	font.SetSize(std::max(8.0f, bounds.Height() * 0.9f));

	if (fRefreshGlyph.exchange(false) || !fHasGlyph || now >= fNextGlyphAt)
		_BeginGlyph(font, now);
	if (fGlyph[0] == '\0') {
		canvas->Sync();
		fOffscreenBitmap->Unlock();
		view->SetDrawingMode(B_OP_COPY);
		view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
		return;
	}

	font_height metrics;
	font.GetHeight(&metrics);
	float glyphWidth = font.StringWidth(fGlyph.data());
	const float maximumWidth = std::max(1.0f, bounds.Width() * 0.9f);
	const float glyphHeight = metrics.ascent + metrics.descent;
	const float maximumHeight = std::max(1.0f, bounds.Height() * 0.9f);
	const float scale = std::min({1.0f,
		glyphWidth > 0.0f ? maximumWidth / glyphWidth : 1.0f,
		glyphHeight > 0.0f ? maximumHeight / glyphHeight : 1.0f});
	if (scale < 1.0f) {
		font.SetSize(font.Size() * scale);
		font.GetHeight(&metrics);
		glyphWidth = font.StringWidth(fGlyph.data());
	}

	const float centreX = (bounds.left + bounds.right) / 2.0f;
	const float centreY = (bounds.top + bounds.bottom) / 2.0f;
	BPoint origin(centreX - glyphWidth / 2.0f,
		centreY + (metrics.ascent - metrics.descent) / 2.0f);

	if (fUseTtfxEffects.load()) {
		const bigtime_t maximumDuration = static_cast<bigtime_t>(
			fIntervalSeconds.load()) * 700000;
		const bigtime_t effectDuration = std::min<bigtime_t>(1500000,
			maximumDuration);
		const double progress = effectDuration > 0
			? std::min(1.0, static_cast<double>(now - fGlyphStarted)
				/ effectDuration)
			: 1.0;
		const double animationTime
			= static_cast<double>(now - fGlyphStarted) / 1000000.0;
		const BRect glyphBounds(origin.x, origin.y - metrics.ascent,
			origin.x + glyphWidth, origin.y + metrics.descent);
		const float blockWidth = glyphBounds.Width() / kBlockColumns;
		const float blockHeight = glyphBounds.Height() / kBlockRows;
		const std::size_t blockCount = kBlockColumns * kBlockRows;
		if (!fGlyphBitmap->Lock()) {
			canvas->Sync();
			fOffscreenBitmap->Unlock();
			return;
		}
		std::memset(fGlyphBitmap->Bits(), 0, fGlyphBitmap->BitsLength());
		fGlyphView->ConstrainClippingRegion(nullptr);
		fGlyphView->SetDrawingMode(B_OP_COPY);
		fGlyphView->SetLowColor(B_TRANSPARENT_COLOR);
		fGlyphView->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE);
		fGlyphView->SetHighColor(fColour.red, fColour.green, fColour.blue,
			fColour.alpha);
		fGlyphView->DrawString(fGlyph.data(), origin);
		fGlyphView->Sync();
		fGlyphBitmap->Unlock();

		canvas->SetDrawingMode(B_OP_ALPHA);
		canvas->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

		std::size_t blockIndex = 0;
		for (int32 row = 0; row < kBlockRows; row++) {
			for (int32 column = 0; column < kBlockColumns;
				column++, blockIndex++) {
				const float left = glyphBounds.left + column * blockWidth;
				const float top = glyphBounds.top + row * blockHeight;
				const float right = column == kBlockColumns - 1
					? glyphBounds.right : left + blockWidth;
				const float bottom = row == kBlockRows - 1
					? glyphBounds.bottom : top + blockHeight;
				const BRect finalBlock(left, top, right, bottom);
				if (!bitmap_block_has_content(fGlyphBitmap, finalBlock))
					continue;
				const ttfx::GlyphState state = fEffectEngine.Transform(blockIndex,
					blockCount, progress, animationTime, finalBlock.left,
					finalBlock.top, bounds.Width(), bounds.Height());
				const float offsetX = static_cast<float>(state.x) - finalBlock.left;
				const float offsetY = static_cast<float>(state.y) - finalBlock.top;
				BRect animatedBlock(finalBlock);
				animatedBlock.OffsetBy(offsetX, offsetY);
				canvas->DrawBitmap(fGlyphBitmap, finalBlock, animatedBlock);
			}
		}
	} else {
		canvas->SetDrawingMode(B_OP_COPY);
		canvas->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE);
		canvas->SetHighColor(fColour.red, fColour.green, fColour.blue,
			fColour.alpha);
		canvas->DrawString(fGlyph.data(), origin);
	}

	canvas->Sync();
	fOffscreenBitmap->Unlock();
	view->SetDrawingMode(B_OP_COPY);
	view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
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
	return fFontIndex.load();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::SelectFont(int32 index)
{
	if (index >= 0 && index < CountFonts()) {
		fFontIndex.store(index);
		fRefreshGlyph.store(true);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
KanjiSaverScreenSaver::IntervalSeconds() const
{
	return fIntervalSeconds.load();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::SetIntervalSeconds(int32 seconds)
{
	fIntervalSeconds.store(kanjisaver::ClampIntervalSeconds(seconds));
	fIntervalChanged.store(true);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
KanjiSaverScreenSaver::UseTtfxEffects() const
{
	return fUseTtfxEffects.load();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::SetUseTtfxEffects(bool enabled)
{
	fUseTtfxEffects.store(enabled);
	fRestartEffect.store(true);
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
				fFontIndex.store(CountFonts() - 1);
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


const KanjiSaverScreenSaver::FontInfo*
KanjiSaverScreenSaver::_SelectedFontInfo() const
{
	const int32 fontIndex = fFontIndex.load();
	if (fontIndex < 0 || fontIndex >= CountFonts())
		return nullptr;
	return &fFonts[fontIndex];
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
KanjiSaverScreenSaver::_ChooseSupportedGlyph(const BFont& font,
	kanjisaver::Colour& colour, kanjisaver::Utf8Glyph& selectedGlyph)
{
	kanjisaver::Utf8Glyph fallback = {};
	selectedGlyph.fill('\0');
	for (int32 attempt = 0; attempt < kGlyphAttempts; attempt++) {
		const kanjisaver::Frame frame = fEngine.NextFrame();
		const kanjisaver::Utf8Glyph glyph
			= kanjisaver::Utf8GlyphForCodePoint(frame.codePoint);
		if (glyph[0] == '\0')
			continue;
		fallback = glyph;
		colour = frame.colour;
		bool hasGlyph = false;
		font.GetHasGlyphs(glyph.data(), 1, &hasGlyph, false);
		if (hasGlyph) {
			selectedGlyph = glyph;
			return true;
		}
	}
	selectedGlyph = fallback;
	return selectedGlyph[0] != '\0';
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::_BeginGlyph(const BFont& font, bigtime_t now)
{
	const bool hadGlyph = fHasGlyph;
	fHasGlyph = _ChooseSupportedGlyph(font, fColour, fGlyph);
	fGlyphStarted = now;
	fNextGlyphAt = now
		+ static_cast<bigtime_t>(fIntervalSeconds.load()) * 1000000;
	if (hadGlyph) {
		fEffectEngine.SetEffect(next_assembly_effect(
			fEffectEngine.CurrentEffect()),
			fEffectRandom());
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
KanjiSaverScreenSaver::_EnsureBuffer(const BRect& bounds)
{
	const BRect bitmapBounds(0.0f, 0.0f, bounds.Width(), bounds.Height());
	if (fOffscreenBitmap != nullptr && fGlyphBitmap != nullptr
		&& fOffscreenBitmap->Bounds() == bitmapBounds) {
		return true;
	}

	delete fOffscreenBitmap;
	delete fGlyphBitmap;
	fOffscreenBitmap = new(std::nothrow) BBitmap(bitmapBounds,
		B_BITMAP_ACCEPTS_VIEWS, B_RGBA32);
	fOffscreenView = nullptr;
	fGlyphBitmap = nullptr;
	fGlyphView = nullptr;
	if (fOffscreenBitmap == nullptr || fOffscreenBitmap->InitCheck() != B_OK) {
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}
	fGlyphBitmap = new(std::nothrow) BBitmap(bitmapBounds,
		B_BITMAP_ACCEPTS_VIEWS, B_RGBA32);
	if (fGlyphBitmap == nullptr || fGlyphBitmap->InitCheck() != B_OK) {
		delete fGlyphBitmap;
		fGlyphBitmap = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}

	fOffscreenView = new(std::nothrow) BView(bitmapBounds,
		"KanjiSaver buffer", B_FOLLOW_NONE, B_WILL_DRAW);
	if (fOffscreenView == nullptr) {
		delete fGlyphBitmap;
		fGlyphBitmap = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}
	fGlyphView = new(std::nothrow) BView(bitmapBounds,
		"KanjiSaver glyph", B_FOLLOW_NONE, B_WILL_DRAW);
	if (fGlyphView == nullptr) {
		delete fOffscreenView;
		fOffscreenView = nullptr;
		delete fGlyphBitmap;
		fGlyphBitmap = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}

	if (!fOffscreenBitmap->Lock()) {
		delete fOffscreenView;
		fOffscreenView = nullptr;
		delete fGlyphView;
		fGlyphView = nullptr;
		delete fGlyphBitmap;
		fGlyphBitmap = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}
	fOffscreenBitmap->AddChild(fOffscreenView);
	fOffscreenView->SetViewColor(B_TRANSPARENT_COLOR);
	fOffscreenBitmap->Unlock();
	if (!fGlyphBitmap->Lock()) {
		delete fGlyphView;
		fGlyphView = nullptr;
		delete fGlyphBitmap;
		fGlyphBitmap = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		fOffscreenView = nullptr;
		return false;
	}
	fGlyphBitmap->AddChild(fGlyphView);
	fGlyphView->SetViewColor(B_TRANSPARENT_COLOR);
	fGlyphBitmap->Unlock();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KanjiSaverScreenSaver::_UpdateTickSize()
{
	const bigtime_t interval = fUseTtfxEffects.load()
		? kTtfxFrameInterval
		: static_cast<bigtime_t>(fIntervalSeconds.load()) * 1000000;
	if (interval != fAppliedTickSize) {
		SetTickSize(interval);
		fAppliedTickSize = interval;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
