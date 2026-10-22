#ifndef TTFX_TEXT_H
#define TTFX_TEXT_H

#include "TTFXEngine.h"

#include <ScreenSaver.h>
#include <String.h>

#include <random>


class BBitmap;
class BView;


class TTFXTextScreenSaver : public BScreenSaver {
public:
	TTFXTextScreenSaver(BMessage* message, image_id image);
	~TTFXTextScreenSaver() override;

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	void _BeginEffect(bigtime_t now);
	bool _EnsureBuffer(const BRect& bounds);
	void _DrawMatrixBackdrop(BView* view, const BRect& bounds, double progress);
	void _DrawText(BView* view, const BRect& bounds, double progress,
		double animationTime);

	ttfx::EffectEngine fEffectEngine;
	std::minstd_rand fRandomEngine;
	BBitmap* fOffscreenBitmap;
	BView* fOffscreenView;
	BString fText;
	bigtime_t fEffectStarted;
	int32 fEffectInterval;
	int32 fTransitionDuration;
	bool fRandomiseEffects;
	bool fPreview;
};


#endif
