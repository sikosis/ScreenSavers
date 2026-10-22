#ifndef TTFX_TIME_H
#define TTFX_TIME_H

#include "TTFXEngine.h"

#include <ScreenSaver.h>

#include <random>


class BView;
class BBitmap;


class TTFXTimeScreenSaver : public BScreenSaver {
public:
	TTFXTimeScreenSaver(BMessage* message, image_id image);
	~TTFXTimeScreenSaver() override;

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	void _BeginEffect(bigtime_t now);
	bool _EnsureBuffer(const BRect& bounds);
	void _DrawClock(BView* view, const BRect& bounds, double progress,
		double animationTime);
	void _DrawMatrixBackdrop(BView* view, const BRect& bounds, double progress);

	ttfx::EffectEngine fEffectEngine;
	std::minstd_rand fRandomEngine;
	BBitmap* fOffscreenBitmap;
	BView* fOffscreenView;
	bigtime_t fEffectStarted;
	int32 fEffectInterval;
	int32 fTransitionDuration;
	bool fShowSeconds;
	bool fUse24HourTime;
	bool fRandomiseEffects;
	bool fPreview;
};


#endif
