#include "TTFXTime.h"

#include "TTFXTimeConfigView.h"

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

	view->SetViewColor(B_TRANSPARENT_COLOR);
	view->SetLowColor(4, 6, 12);
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
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
	const BRect bounds = view->Bounds();
	view->SetLowColor(4, 6, 12);
	view->FillRect(bounds, B_SOLID_LOW);

	if (fEffectEngine.CurrentEffect() == ttfx::Effect::kMatrix)
		_DrawMatrixBackdrop(view, bounds, progress);
	_DrawClock(view, bounds, progress);
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


void
TTFXTimeScreenSaver::_DrawClock(BView* view, const BRect& bounds, double progress)
{
	const std::string clock = format_time(fShowSeconds, fUse24HourTime);
	if (clock.empty())
		return;

	BFont font(be_bold_font);
	const float availableWidth = bounds.Width() * (fPreview ? 0.84f : 0.78f);
	const float estimatedSize = availableWidth
		/ static_cast<float>(clock.size()) * 1.52f;
	const float fontSize = std::max(18.0f,
		std::min(estimatedSize, bounds.Height() * (fPreview ? 0.34f : 0.28f)));
	font.SetSize(fontSize);

	const float textWidth = font.StringWidth(clock.c_str());
	font_height metrics;
	font.GetHeight(&metrics);
	const float baseline = bounds.top + bounds.Height() / 2.0f
		+ (metrics.ascent - metrics.descent) / 2.0f;
	float cursor = bounds.left + (bounds.Width() - textWidth) / 2.0f;

	for (std::size_t index = 0; index < clock.size(); index++) {
		char glyphText[2] = {clock[index], '\0'};
		const float glyphWidth = font.StringWidth(glyphText);
		const ttfx::GlyphState state = fEffectEngine.Transform(index, clock.size(),
			progress, cursor, baseline, bounds.Width(), bounds.Height());

		BFont glyphFont(font);
		glyphFont.SetSize(fontSize * static_cast<float>(state.scale));
		view->SetFont(&glyphFont);
		view->SetHighColor(state.colour.red, state.colour.green, state.colour.blue,
			static_cast<uint8>(std::lround(state.opacity * 255.0)));
		if (state.replacement != '\0')
			glyphText[0] = state.replacement;
		view->DrawString(glyphText, BPoint(static_cast<float>(state.x),
			static_cast<float>(state.y)));
		cursor += glyphWidth;
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
		baseline + metrics.descent + fontSize * 0.3f));
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
