#include "BinaryClock.h"

#include "BinaryClockConfigView.h"
#include "BinaryClockEngine.h"

#include <Bitmap.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <new>
#include <vector>


namespace {

const char* const kEffectIntervalKey = "effect_interval";
const char* const kTransitionDurationKey = "transition_duration";
const char* const kShowSecondsKey = "show_seconds";
const char* const kUse24HourTimeKey = "use_24_hour_time";
const char* const kShowBitWeightsKey = "show_bit_weights";
const char* const kShowGroupLabelsKey = "show_group_labels";
const char* const kUseTtfxEffectsKey = "use_ttfx_effects";
const char* const kRandomiseEffectsKey = "randomise_effects";

const bigtime_t kEffectFrameInterval = 33333;
const bigtime_t kClockFrameInterval = 250000;
const int32 kDefaultEffectInterval = 15;
const int32 kDefaultTransitionDuration = 3;
const int32 kBackgroundRed = 5;
const int32 kBackgroundGreen = 8;
const int32 kBackgroundBlue = 16;


void
current_local_time(int& hour, int& minute, int& second)
{
	const std::time_t now = std::time(nullptr);
	std::tm local = {};
#if defined(__HAIKU__)
	localtime_r(&now, &local);
#else
	local = *std::localtime(&now);
#endif
	hour = local.tm_hour;
	minute = local.tm_min;
	second = local.tm_sec;
}
//---------------------------------------------------------------------------------------------------------------------------------//


ttfx::Colour
group_colour(int column)
{
	switch (binaryclock::ColumnGroup(column)) {
		case 0:
			return {255, 176, 69};
		case 1:
			return {64, 224, 255};
		case 2:
			return {112, 255, 170};
		default:
			return {180, 200, 230};
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint8
alpha_for(double opacity)
{
	return static_cast<uint8>(std::lround(
		std::clamp(opacity, 0.0, 1.0) * 255.0));
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* message, image_id image)
{
	return new(std::nothrow) BinaryClockScreenSaver(message, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BinaryClockScreenSaver::BinaryClockScreenSaver(BMessage* message,
	image_id image)
	:
	BScreenSaver(message, image),
	fRandomEngine(static_cast<uint32>(system_time())),
	fOffscreenBitmap(nullptr),
	fOffscreenView(nullptr),
	fEffectStarted(0),
	fAppliedTickSize(0),
	fEffectInterval(kDefaultEffectInterval),
	fTransitionDuration(kDefaultTransitionDuration),
	fShowSeconds(true),
	fUse24HourTime(true),
	fShowBitWeights(true),
	fShowGroupLabels(true),
	fUseTtfxEffects(false),
	fRandomiseEffects(true),
	fPreview(false)
{
	if (message != nullptr) {
		message->FindInt32(kEffectIntervalKey, &fEffectInterval);
		message->FindInt32(kTransitionDurationKey, &fTransitionDuration);
		message->FindBool(kShowSecondsKey, &fShowSeconds);
		message->FindBool(kUse24HourTimeKey, &fUse24HourTime);
		message->FindBool(kShowBitWeightsKey, &fShowBitWeights);
		message->FindBool(kShowGroupLabelsKey, &fShowGroupLabels);
		message->FindBool(kUseTtfxEffectsKey, &fUseTtfxEffects);
		message->FindBool(kRandomiseEffectsKey, &fRandomiseEffects);
	}
	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BinaryClockScreenSaver::~BinaryClockScreenSaver()
{
	delete fOffscreenBitmap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BinaryClockScreenSaver::SaveState(BMessage* into) const
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
		status = into->AddBool(kShowBitWeightsKey, fShowBitWeights);
	if (status == B_OK)
		status = into->AddBool(kShowGroupLabelsKey, fShowGroupLabels);
	if (status == B_OK)
		status = into->AddBool(kUseTtfxEffectsKey, fUseTtfxEffects);
	if (status == B_OK)
		status = into->AddBool(kRandomiseEffectsKey, fRandomiseEffects);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;

	view->AddChild(new BinaryClockConfigView(view->Bounds(), &fEffectInterval,
		&fTransitionDuration, &fShowSeconds, &fUse24HourTime, &fShowBitWeights,
		&fShowGroupLabels, &fUseTtfxEffects, &fRandomiseEffects));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BinaryClockScreenSaver::StartSaver(BView* view, bool preview)
{
	if (view == nullptr)
		return B_BAD_VALUE;

	_ApplySettings();
	fPreview = preview;
	fEffectStarted = 0;
	fAppliedTickSize = 0;
	fEffectEngine.SetEffect(ttfx::Effect::kExpand, fRandomEngine());

	view->SetViewColor(kBackgroundRed, kBackgroundGreen, kBackgroundBlue);
	view->SetLowColor(kBackgroundRed, kBackgroundGreen, kBackgroundBlue);
	view->FillRect(view->Bounds(), B_SOLID_LOW);
	_UpdateTickSize();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::StopSaver()
{
	delete fOffscreenBitmap;
	fOffscreenBitmap = nullptr;
	fOffscreenView = nullptr;
	fAppliedTickSize = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	_ApplySettings();
	_UpdateTickSize();
	const bigtime_t now = system_time();
	if (fUseTtfxEffects) {
		const bigtime_t interval = static_cast<bigtime_t>(fEffectInterval)
			* 1000000;
		if (fEffectStarted <= 0 || now - fEffectStarted >= interval)
			_BeginEffect(now);
	}

	const BRect targetBounds = view->Bounds();
	if (!_EnsureBuffer(targetBounds))
		return;
	if (!fOffscreenBitmap->Lock())
		return;

	const BRect bounds = fOffscreenBitmap->Bounds();
	BView* canvas = fOffscreenView;
	canvas->ConstrainClippingRegion(nullptr);
	canvas->SetDrawingMode(B_OP_COPY);
	canvas->SetLowColor(kBackgroundRed, kBackgroundGreen, kBackgroundBlue, 255);
	canvas->FillRect(bounds, B_SOLID_LOW);
	canvas->SetDrawingMode(B_OP_ALPHA);
	canvas->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);

	double progress = 1.0;
	double animationTime = 0.0;
	if (fUseTtfxEffects) {
		progress = fTransitionDuration > 0
			? std::min(1.0, static_cast<double>(now - fEffectStarted)
				/ (static_cast<double>(fTransitionDuration) * 1000000.0))
			: 1.0;
		animationTime = static_cast<double>(now - fEffectStarted) / 1000000.0;
		if (fEffectEngine.CurrentEffect() == ttfx::Effect::kMatrix)
			_DrawMatrixBackdrop(canvas, bounds, progress);
	}

	_DrawClock(canvas, bounds, progress, animationTime);
	canvas->Sync();
	fOffscreenBitmap->Unlock();

	view->SetDrawingMode(B_OP_COPY);
	view->DrawBitmap(fOffscreenBitmap, bounds, targetBounds);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_ApplySettings()
{
	fEffectInterval = std::clamp(fEffectInterval, 5, 60);
	fTransitionDuration = std::clamp(fTransitionDuration, 1, 5);
	if (fTransitionDuration >= fEffectInterval)
		fTransitionDuration = std::max(1, fEffectInterval - 1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_BeginEffect(bigtime_t now)
{
	ttfx::Effect next = ttfx::EffectEngine::NextEffect(
		fEffectEngine.CurrentEffect());
	if (fRandomiseEffects) {
		std::uniform_int_distribution<int32> effect(0,
			static_cast<int32>(ttfx::Effect::kCount) - 1);
		do {
			next = static_cast<ttfx::Effect>(effect(fRandomEngine));
		} while (next == fEffectEngine.CurrentEffect()
			&& static_cast<int>(ttfx::Effect::kCount) > 1);
	}
	fEffectEngine.SetEffect(next, fRandomEngine());
	fEffectStarted = now;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_UpdateTickSize()
{
	const bigtime_t interval = fUseTtfxEffects
		? kEffectFrameInterval : kClockFrameInterval;
	if (interval != fAppliedTickSize) {
		SetTickSize(interval);
		fAppliedTickSize = interval;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
BinaryClockScreenSaver::_EnsureBuffer(const BRect& bounds)
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

	fOffscreenView = new(std::nothrow) BView(bitmapBounds, "BinaryClock buffer",
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
BinaryClockScreenSaver::_DrawClock(BView* view, const BRect& bounds,
	double progress, double animationTime)
{
	int hour = 0;
	int minute = 0;
	int second = 0;
	current_local_time(hour, minute, second);
	const binaryclock::ClockDigits clock = binaryclock::DecodeClock(hour,
		minute, second, fShowSeconds, fUse24HourTime);
	std::vector<binaryclock::BitCell> cells;
	binaryclock::BuildBitCells(clock, cells);
	if (cells.empty())
		return;

	const float columnSpan = static_cast<float>(
		binaryclock::ColumnPosition(clock.count - 1)) + 1.0f;
	const float rowSpan = static_cast<float>(binaryclock::kLedRows);
	const float labelWidth = fShowBitWeights ? 1.35f : 0.0f;
	const float labelHeight = fShowGroupLabels ? 1.15f : 0.0f;
	const float totalWidth = columnSpan + labelWidth;
	const float totalHeight = rowSpan + labelHeight;
	const float availableWidth = std::max(1.0f,
		bounds.Width() * (fPreview ? 0.9f : 0.84f));
	const float availableHeight = std::max(1.0f,
		bounds.Height() * (fPreview ? 0.86f : 0.8f));
	const float pitch = std::max(2.0f,
		std::min(availableWidth / totalWidth, availableHeight / totalHeight));
	const float contentWidth = totalWidth * pitch;
	const float contentHeight = totalHeight * pitch;
	const float contentLeft = bounds.left
		+ (bounds.Width() - contentWidth) / 2.0f;
	const float contentTop = bounds.top
		+ (bounds.Height() - contentHeight) / 2.0f;
	const float gridLeft = contentLeft + labelWidth * pitch;
	const float gridTop = contentTop;

	const bool effects = fUseTtfxEffects;
	std::size_t cellIndex = 0;
	for (const binaryclock::BitCell& cell : cells) {
		const float finalX = gridLeft
			+ (static_cast<float>(binaryclock::ColumnPosition(cell.column))
				+ 0.5f) * pitch;
		const float finalY = gridTop + (cell.row + 0.5f) * pitch;
		const std::size_t effectIndex = cellIndex++;
		if (!cell.lit) {
			_DrawSocket(view, finalX, finalY, pitch * 0.31f);
			continue;
		}

		float x = finalX;
		float y = finalY;
		double scale = 1.0;
		double opacity = 1.0;
		ttfx::Colour colour = group_colour(cell.column);
		char replacement = '\0';
		if (effects) {
			const ttfx::GlyphState state = fEffectEngine.Transform(effectIndex,
				cells.size(), progress, animationTime, finalX, finalY,
				bounds.Width(), bounds.Height());
			x = static_cast<float>(state.x);
			y = static_cast<float>(state.y);
			scale = state.scale;
			opacity = state.opacity;
			colour = state.colour;
			replacement = state.replacement;
		}

		const float radius = std::max(1.0f,
			pitch * 0.33f * static_cast<float>(scale));
		_DrawLed(view, x, y, radius, colour, opacity, replacement);
	}

	if (fShowBitWeights || fShowGroupLabels)
		_DrawLabels(view, gridLeft, gridTop, pitch, clock.count);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_DrawLabels(BView* view, float gridLeft, float gridTop,
	float pitch, int columns)
{
	const float gridBottom = gridTop
		+ static_cast<float>(binaryclock::kLedRows) * pitch;

	if (fShowBitWeights) {
		BFont font(be_plain_font);
		font.SetSize(std::max(7.0f, pitch * 0.32f));
		view->SetFont(&font);
		view->SetHighColor(128, 150, 180, 190);
		for (int row = 0; row < binaryclock::kLedRows; row++) {
			const int weight = 1 << (binaryclock::kLedRows - 1 - row);
			const char text[2] = {static_cast<char>('0' + weight), '\0'};
			const float width = font.StringWidth(text);
			const float y = gridTop + (row + 0.5f) * pitch
				+ font.Size() * 0.35f;
			view->DrawString(text, BPoint(gridLeft - pitch * 0.32f - width, y));
		}
	}

	if (fShowGroupLabels) {
		BFont font(be_bold_font);
		font.SetSize(std::max(7.0f, pitch * 0.36f));
		view->SetFont(&font);
		view->SetHighColor(150, 172, 205, 200);
		const char* labels = "HMS";
		for (int group = 0; group < columns / 2; group++) {
			const int firstColumn = group * 2;
			const float first = static_cast<float>(
				binaryclock::ColumnPosition(firstColumn));
			const float last = static_cast<float>(
				binaryclock::ColumnPosition(firstColumn + 1));
			const float centreX = gridLeft
				+ ((first + last) / 2.0f + 0.5f) * pitch;
			const char text[2] = {labels[group], '\0'};
			const float width = font.StringWidth(text);
			const float y = gridBottom + pitch * 0.82f;
			view->DrawString(text, BPoint(centreX - width / 2.0f, y));
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_DrawSocket(BView* view, float x, float y,
	float radius)
{
	if (radius < 0.5f)
		return;

	const BRect rect(x - radius, y - radius, x + radius, y + radius);
	view->SetHighColor(26, 36, 52, 200);
	view->FillEllipse(rect);
	view->SetHighColor(44, 60, 84, 150);
	view->StrokeEllipse(rect);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_DrawLed(BView* view, float x, float y, float radius,
	const ttfx::Colour& colour, double opacity, char replacement)
{
	if (radius < 0.5f || opacity <= 0.01)
		return;

	const uint8 alpha = alpha_for(opacity);
	if (replacement != '\0') {
		BFont font(be_fixed_font);
		font.SetSize(std::max(7.0f, radius * 2.4f));
		view->SetFont(&font);
		font_height metrics;
		font.GetHeight(&metrics);
		const char text[2] = {replacement, '\0'};
		const float width = font.StringWidth(text);
		view->SetHighColor(colour.red, colour.green, colour.blue, alpha);
		view->DrawString(text, BPoint(x - width / 2.0f, y
			+ (metrics.ascent - metrics.descent) / 2.0f));
		return;
	}

	const float halo = radius * 1.85f;
	view->SetHighColor(colour.red, colour.green, colour.blue,
		static_cast<uint8>(alpha / 6));
	view->FillEllipse(BRect(x - halo, y - halo, x + halo, y + halo));
	view->SetHighColor(colour.red, colour.green, colour.blue, alpha);
	view->FillEllipse(BRect(x - radius, y - radius, x + radius, y + radius));

	const float shine = std::max(0.5f, radius * 0.34f);
	const float shineX = x - radius * 0.32f;
	const float shineY = y - radius * 0.32f;
	view->SetHighColor(255, 255, 255, static_cast<uint8>(alpha / 4));
	view->FillEllipse(BRect(shineX - shine, shineY - shine,
		shineX + shine, shineY + shine));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BinaryClockScreenSaver::_DrawMatrixBackdrop(BView* view, const BRect& bounds,
	double progress)
{
	const int32 columns = std::max(8,
		static_cast<int32>(bounds.Width() / 48.0f));
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
			static_cast<uint8>(std::lround(45.0 * progress)));
		view->DrawString(digit, BPoint(x, y));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
