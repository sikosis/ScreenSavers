/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef BE_WAKE_H
#define BE_WAKE_H

#include "BeWakeEngine.h"

#include <ScreenSaver.h>


class BBitmap;
class BView;


class BeWakeScreenSaver : public BScreenSaver {
public:
	BeWakeScreenSaver(BMessage* archive, image_id image);
	~BeWakeScreenSaver() override;

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void StopSaver() override;
	void Draw(BView* view, int32 frame) override;

private:
	void _ApplySettings();
	bool _EnsureBuffer(const BRect& bounds);
	void _SynchroniseEngine();

	bewake::Engine fEngine;
	BBitmap* fBackBitmap;
	BView* fBackView;
	float fColour;
	bool fCycleColours;
	bool fRandomColour;
	bool fWantReset;
};


#endif
