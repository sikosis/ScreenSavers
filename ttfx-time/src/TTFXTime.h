#ifndef TTFX_TIME_H
#define TTFX_TIME_H

#include "TTFXEngine.h"

#include <ScreenSaver.h>

#include <random>


class BView;


class TTFXTimeScreenSaver : public BScreenSaver {
public:
	TTFXTimeScreenSaver(BMessage* message, image_id image);

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	void _BeginEffect(bigtime_t now);
	void _DrawClock(BView* view, const BRect& bounds, double progress);
	void _DrawMatrixBackdrop(BView* view, const BRect& bounds, double progress);

	ttfx::EffectEngine fEffectEngine;
	std::minstd_rand fRandomEngine;
	bigtime_t fEffectStarted;
	int32 fEffectInterval;
	int32 fTransitionDuration;
	bool fShowSeconds;
	bool fUse24HourTime;
	bool fRandomiseEffects;
	bool fPreview;
};


#endif
