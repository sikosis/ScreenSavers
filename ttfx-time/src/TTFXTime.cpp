#include "TTFXTime.h"

#include "TTFXClock.h"
#include "TTFXTimeConfigView.h"

#include <Bitmap.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <new>
#include <string>


namespace {

const char* const kEffectIntervalKey = "effect_interval";
const char* const kTransitionDurationKey = "transition_duration";
const char* const kShowSecondsKey = "show_seconds";
const char* const kUse24HourTimeKey = "use_24_hour_time";
const char* const kRandomiseEffectsKey = "randomise_effects";

const bigtime_t kFrameInterval = 16667;
const int32 kClockRows = 7;


std::string
format_time(bool showSeconds, bool use24HourTime)
{
	const std::time_t now = std::time(nullptr);
	std::tm local = {};
#if defined(__HAIKU__)
	localtime_r(&now, &local);
#else
	local = *std::localtime(&now);
#endif
	char text[32];
	const char* format = use24HourTime
		? (showSeconds ? "%H:%M:%S" : "%H:%M")
		: (showSeconds ? "%I:%M:%S" : "%I:%M");
	std::strftime(text, sizeof(text), format, &local);
	if (!use24HourTime && text[0] == '0')
		return text + 1;
	return text;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* message, image_id image)
{
	return new(std::nothrow) TTFXTimeScreenSaver(message, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


TTFXTimeScreenSaver::TTFXTimeScreenSaver(BMessage* message, image_id image)
	:
	BScreenSaver(message, image),
	fRandomEngine(static_cast<uint32>(system_time())),
	fOffscreenBitmap(nullptr),
	fOffscreenView(nullptr),
	fEffectStarted(0),
	fEffectInterval(12),
	fTransitionDuration(3),
	fShowSeconds(true),
	fUse24HourTime(true),
	fRandomiseEffects(true),
	fPreview(false)
{
	if (message != nullptr) {
		message->FindInt32(kEffectIntervalKey, &fEffectInterval);
		message->FindInt32(kTransitionDurationKey, &fTransitionDuration);
		message->FindBool(kShowSecondsKey, &fShowSeconds);
		message->FindBool(kUse24HourTimeKey, &fUse24HourTime);
		message->FindBool(kRandomiseEffectsKey, &fRandomiseEffects);
	}
	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


TTFXTimeScreenSaver::~TTFXTimeScreenSaver()
{
	delete fOffscreenBitmap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TTFXTimeScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddInt32(kEffectIntervalKey, fEffectInterval);
	if (status == B_OK)
		status = into->AddInt32(kTransitionDurationKey, fTransitionDuration);
	if (status == B_OK)
		status = into->AddBool(kShowSecondsKey, fShowSeconds);
	if (status == B_OK)
		status = into->AddBool(kUse24HourTimeKey, fUse24HourTime);
	if (status == B_OK)
		status = into->AddBool(kRandomiseEffectsKey, fRandomiseEffects);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;

	view->AddChild(new TTFXTimeConfigView(view->Bounds(), &fEffectInterval,
		&fTransitionDuration, &fShowSeconds, &fUse24HourTime,
		&fRandomiseEffects));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TTFXTimeScreenSaver::StartSaver(BView* view, bool preview)
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
TTFXTimeScreenSaver::Draw(BView* view, int32)
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
	_DrawClock(fOffscreenView, bounds, progress);
	fOffscreenView->Sync();
	fOffscreenBitmap->Unlock();

	view->SetDrawingMode(B_OP_COPY);
	view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeScreenSaver::_ApplySettings()
{
	fEffectInterval = std::clamp(fEffectInterval, 5, 60);
	fTransitionDuration = std::clamp(fTransitionDuration, 1, 5);
	if (fTransitionDuration >= fEffectInterval)
		fTransitionDuration = std::max(1, fEffectInterval - 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeScreenSaver::_BeginEffect(bigtime_t now)
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
TTFXTimeScreenSaver::_EnsureBuffer(const BRect& bounds)
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

	fOffscreenView = new(std::nothrow) BView(bitmapBounds, "ttfx-time buffer",
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
TTFXTimeScreenSaver::_DrawClock(BView* view, const BRect& bounds, double progress)
{
	const std::string clock = format_time(fShowSeconds, fUse24HourTime);
	if (clock.empty())
		return;

	std::vector<ttfx::ClockCell> cells;
	const int32 totalColumns = ttfx::BuildClockCells(clock, cells);
	if (cells.empty() || totalColumns <= 0)
		return;

	BFont font(be_fixed_font);
	const float availableWidth = bounds.Width() * (fPreview ? 0.84f : 0.78f);
	const float availableHeight = bounds.Height() * (fPreview ? 0.64f : 0.58f);
	float fontSize = std::max(5.0f, availableHeight / kClockRows * 0.78f);
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
	const float artHeight = kClockRows * lineHeight;
	const float left = bounds.left + (bounds.Width() - artWidth) / 2.0f;
	const float artTop = bounds.top + (bounds.Height() - artHeight) / 2.0f;
	const float firstBaseline = artTop + metrics.ascent;

	for (std::size_t index = 0; index < cells.size(); index++) {
		const float x = left + cells[index].column * cellWidth;
		const float y = firstBaseline + cells[index].row * lineHeight;
		const ttfx::GlyphState state = fEffectEngine.Transform(index, cells.size(),
			progress, x, y, bounds.Width(), bounds.Height());

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
//---------------------------------------------------------------------------------------------------------------------------------//


void
TTFXTimeScreenSaver::_DrawMatrixBackdrop(BView* view, const BRect& bounds,
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
