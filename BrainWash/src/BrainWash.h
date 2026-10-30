/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#ifndef BRAIN_WASH_H
#define BRAIN_WASH_H

#include "BrainWashEngine.h"

#include <ScreenSaver.h>


class BBitmap;


class BrainWashScreenSaver : public BScreenSaver {
public:
	BrainWashScreenSaver(BMessage* archive, image_id image);
	~BrainWashScreenSaver() override;

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void StopSaver() override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	status_t _ConfigureCanvas(BView* view);
	void _RenderBitmap();

	brainwash::Engine fEngine;
	BBitmap* fBitmap;
	bigtime_t fPatternCompletedAt;
	int32 fCycleSpeed;
	int32 fDrawingGap;
	int32 fDrawingSpeed;
};


#endif
