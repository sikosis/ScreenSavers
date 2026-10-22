#include "TTFXText.h"

#include "TTFXTextArt.h"
#include "TTFXTextConfigView.h"

#include <Bitmap.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <new>
#include <vector>


namespace {

const char* const kTextKey = "text";
const char* const kEffectIntervalKey = "effect_interval";
const char* const kTransitionDurationKey = "transition_duration";
const char* const kRandomiseEffectsKey = "randomise_effects";
const char* const kShowEffectNameKey = "show_effect_name";

const bigtime_t kFrameInterval = 16667;
const int32 kTextRows = 7;

}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* message, image_id image)
{
	return new(std::nothrow) TTFXTextScreenSaver(message, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


TTFXTextScreenSaver::TTFXTextScreenSaver(BMessage* message, image_id image)
	:
	BScreenSaver(message, image),
	fRandomEngine(static_cast<uint32>(system_time())),
	fOffscreenBitmap(nullptr),
	fOffscreenView(nullptr),
	fText("HAIKU"),
	fEffectStarted(0),
	fEffectInterval(12),
	fTransitionDuration(3),
	fRandomiseEffects(true),
	fShowEffectName(false),
	fPreview(false)
{
	if (message != nullptr) {
		const char* text = nullptr;
		if (message->FindString(kTextKey, &text) == B_OK && text != nullptr)
			fText = text;
		message->FindInt32(kEffectIntervalKey, &fEffectInterval);
		message->FindInt32(kTransitionDurationKey, &fTransitionDuration);
		message->FindBool(kRandomiseEffectsKey, &fRandomiseEffects);
		message->FindBool(kShowEffectNameKey, &fShowEffectName);
	}
	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


TTFXTextScreenSaver::~TTFXTextScreenSaver()
{
	delete fOffscreenBitmap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TTFXTextScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddString(kTextKey, fText.String());
	if (status == B_OK)
		status = into->AddInt32(kEffectIntervalKey, fEffectInterval);
	if (status == B_OK)
		status = into->AddInt32(kTransitionDurationKey, fTransitionDuration);
	if (status == B_OK)
		status = into->AddBool(kRandomiseEffectsKey, fRandomiseEffects);
	if (status == B_OK)
		status = into->AddBool(kShowEffectNameKey, fShowEffectName);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;

	view->AddChild(new TTFXTextConfigView(view->Bounds(), &fText,
		&fEffectInterval, &fTransitionDuration, &fRandomiseEffects,
		&fShowEffectName));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TTFXTextScreenSaver::StartSaver(BView* view, bool preview)
{
	if (view == nullptr)
		return B_BAD_VALUE;

	_ApplySettings();
	fPreview = preview;
	fEffectEngine.SetEffect(ttfx::Effect::kExpand, fRandomEngine());
	_BeginEffect(system_time());

	view->SetViewColor(4, 6, 12);
	view->SetLowColor(4, 6, 12);
	view->FillRect(view->Bounds(), B_SOLID_LOW);
	SetTickSize(kFrameInterval);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	_ApplySettings();
	const bigtime_t now = system_time();
	const bigtime_t interval = static_cast<bigtime_t>(fEffectInterval) * 1000000;
	if (fEffectStarted <= 0 || now - fEffectStarted >= interval)
		_BeginEffect(now);

	const double progress = std::min(1.0,
		static_cast<double>(now - fEffectStarted)
			/ (static_cast<double>(fTransitionDuration) * 1000000.0));
	const double animationTime = static_cast<double>(now - fEffectStarted) / 1000000.0;
	const BRect targetBounds = view->Bounds();
	if (!_EnsureBuffer(targetBounds))
		return;
	if (!fOffscreenBitmap->Lock())
		return;

	const BRect bounds = fOffscreenBitmap->Bounds();
	fOffscreenView->SetLowColor(4, 6, 12);
	fOffscreenView->FillRect(bounds, B_SOLID_LOW);
	if (fEffectEngine.CurrentEffect() == ttfx::Effect::kMatrix)
		_DrawMatrixBackdrop(fOffscreenView, bounds, progress);
	_DrawText(fOffscreenView, bounds, progress, animationTime);
	fOffscreenView->Sync();
	fOffscreenBitmap->Unlock();

	view->SetDrawingMode(B_OP_COPY);
	view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::_ApplySettings()
{
	fEffectInterval = std::clamp(fEffectInterval, 5, 60);
	fTransitionDuration = std::clamp(fTransitionDuration, 1, 5);
	if (fTransitionDuration >= fEffectInterval)
		fTransitionDuration = std::max(1, fEffectInterval - 1);
	if (fText.IsEmpty())
		fText = "HAIKU";
	if (fText.Length() > 16)
		fText.Truncate(16);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::_BeginEffect(bigtime_t now)
{
	ttfx::Effect next = ttfx::EffectEngine::NextEffect(fEffectEngine.CurrentEffect());
	if (fRandomiseEffects) {
		std::uniform_int_distribution<int32> effect(0,
			static_cast<int32>(ttfx::Effect::kCount) - 1);
		do {
			next = static_cast<ttfx::Effect>(effect(fRandomEngine));
		} while (next == fEffectEngine.CurrentEffect());
	}
	fEffectEngine.SetEffect(next, fRandomEngine());
	fEffectStarted = now;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TTFXTextScreenSaver::_EnsureBuffer(const BRect& bounds)
{
	const BRect bitmapBounds(0.0f, 0.0f, bounds.Width(), bounds.Height());
	if (fOffscreenBitmap != nullptr
		&& fOffscreenBitmap->Bounds() == bitmapBounds) {
		return true;
	}

	delete fOffscreenBitmap;
	fOffscreenBitmap = new(std::nothrow) BBitmap(bitmapBounds,
		B_BITMAP_ACCEPTS_VIEWS, B_RGBA32);
	fOffscreenView = nullptr;
	if (fOffscreenBitmap == nullptr || fOffscreenBitmap->InitCheck() != B_OK) {
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}

	fOffscreenView = new(std::nothrow) BView(bitmapBounds, "ttfx-text buffer",
		B_FOLLOW_NONE, B_WILL_DRAW);
	if (fOffscreenView == nullptr) {
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}

	if (!fOffscreenBitmap->Lock()) {
		delete fOffscreenView;
		fOffscreenView = nullptr;
		delete fOffscreenBitmap;
		fOffscreenBitmap = nullptr;
		return false;
	}
	fOffscreenBitmap->AddChild(fOffscreenView);
	fOffscreenView->SetViewColor(B_TRANSPARENT_COLOR);
	fOffscreenView->SetDrawingMode(B_OP_ALPHA);
	fOffscreenView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
	fOffscreenBitmap->Unlock();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::_DrawText(BView* view, const BRect& bounds,
	double progress, double animationTime)
{
	std::vector<ttfx::TextCell> cells;
	const int32 totalColumns = ttfx::BuildTextCells(fText.String(), cells);
	if (cells.empty() || totalColumns <= 0)
		return;

	BFont font(be_fixed_font);
	const float availableWidth = bounds.Width() * (fPreview ? 0.84f : 0.78f);
	const float availableHeight = bounds.Height() * (fPreview ? 0.64f : 0.58f);
	float fontSize = std::max(5.0f, availableHeight / kTextRows * 0.78f);
	font.SetSize(fontSize);

	font_height metrics;
	font.GetHeight(&metrics);
	const char* const blockText = "\xE2\x96\x88\xE2\x96\x88";
	float cellWidth = font.StringWidth(blockText);
	float lineHeight = metrics.ascent + metrics.descent + metrics.leading;
	const float initialWidth = totalColumns * cellWidth;
	if (initialWidth > availableWidth) {
		fontSize *= availableWidth / initialWidth;
		font.SetSize(fontSize);
		font.GetHeight(&metrics);
		cellWidth = font.StringWidth(blockText);
		lineHeight = metrics.ascent + metrics.descent + metrics.leading;
	}

	const float artWidth = totalColumns * cellWidth;
	const float artHeight = kTextRows * lineHeight;
	const float left = bounds.left + (bounds.Width() - artWidth) / 2.0f;
	const float artTop = bounds.top + (bounds.Height() - artHeight) / 2.0f;
	const float firstBaseline = artTop + metrics.ascent;

	for (std::size_t index = 0; index < cells.size(); index++) {
		const float x = left + cells[index].column * cellWidth;
		const float y = firstBaseline + cells[index].row * lineHeight;
		const ttfx::GlyphState state = fEffectEngine.Transform(index, cells.size(),
			progress, animationTime, x, y, bounds.Width(), bounds.Height());

		BFont glyphFont(font);
		glyphFont.SetSize(fontSize * static_cast<float>(state.scale));
		view->SetFont(&glyphFont);
		view->SetHighColor(state.colour.red, state.colour.green, state.colour.blue,
			static_cast<uint8>(std::lround(state.opacity * 255.0)));
		const char* block = blockText;
		char replacement[3] = {'\0', '\0', '\0'};
		if (state.replacement != '\0') {
			replacement[0] = state.replacement;
			replacement[1] = state.replacement;
			block = replacement;
		}
		view->DrawString(block, BPoint(static_cast<float>(state.x),
			static_cast<float>(state.y)));
	}

	if (fShowEffectName) {
		BFont labelFont(be_plain_font);
		labelFont.SetSize(std::max(9.0f, fontSize * 0.095f));
		view->SetFont(&labelFont);
		const char* effectName = ttfx::EffectEngine::EffectName(
			fEffectEngine.CurrentEffect());
		const float labelWidth = labelFont.StringWidth(effectName);
		view->SetHighColor(166, 180, 205,
			static_cast<uint8>(std::lround(std::min(1.0, progress * 2.0) * 210.0)));
		view->DrawString(effectName, BPoint(bounds.left
			+ (bounds.Width() - labelWidth) / 2.0f,
			artTop + artHeight + fontSize * 0.35f));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTextScreenSaver::_DrawMatrixBackdrop(BView* view, const BRect& bounds,
	double progress)
{
	const int32 columns = std::max(8, static_cast<int32>(bounds.Width() / 48.0f));
	BFont font(be_fixed_font);
	font.SetSize(std::max(9.0f, bounds.Height() / 60.0f));
	view->SetFont(&font);
	for (int32 column = 0; column < columns; column++) {
		const uint32 value = static_cast<uint32>(column * 2654435761U
			+ static_cast<uint32>(system_time() / 80000));
		const float x = bounds.left + (column + 0.5f) * bounds.Width() / columns;
		const float y = bounds.top + static_cast<float>(value % 1000) / 1000.0f
			* bounds.Height();
		const char digit[2] = {(value & 1) == 0 ? '0' : '1', '\0'};
		view->SetHighColor(35, 210, 92,
			static_cast<uint8>(std::lround(70.0 * progress)));
		view->DrawString(digit, BPoint(x, y));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
