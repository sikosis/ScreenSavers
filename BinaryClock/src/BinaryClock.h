#ifndef BINARY_CLOCK_H
#define BINARY_CLOCK_H

#include "TTFXEngine.h"

#include <ScreenSaver.h>

#include <random>


class BView;
class BBitmap;
class BRect;


class BinaryClockScreenSaver : public BScreenSaver {
public:
	BinaryClockScreenSaver(BMessage* message, image_id image);
	~BinaryClockScreenSaver() override;

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void StopSaver() override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	void _BeginEffect(bigtime_t now);
	void _UpdateTickSize();
	bool _EnsureBuffer(const BRect& bounds);
	void _DrawClock(BView* view, const BRect& bounds, double progress,
		double animationTime);
	void _DrawLabels(BView* view, float gridLeft, float gridTop, float pitch,
		int columns);
	void _DrawSocket(BView* view, float x, float y, float radius);
	void _DrawLed(BView* view, float x, float y, float radius,
		const ttfx::Colour& colour, double opacity, char replacement);
	void _DrawMatrixBackdrop(BView* view, const BRect& bounds, double progress);

	ttfx::EffectEngine fEffectEngine;
	std::minstd_rand fRandomEngine;
	BBitmap* fOffscreenBitmap;
	BView* fOffscreenView;
	bigtime_t fEffectStarted;
	bigtime_t fAppliedTickSize;
	int32 fEffectInterval;
	int32 fTransitionDuration;
	bool fShowSeconds;
	bool fUse24HourTime;
	bool fShowBitWeights;
	bool fShowGroupLabels;
	bool fUseTtfxEffects;
	bool fRandomiseEffects;
	bool fPreview;
};


#endif
