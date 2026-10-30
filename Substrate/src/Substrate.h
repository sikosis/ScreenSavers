/*
 * Copyright (c) 2014 Josef Gajdusek
 * Based on XScreenSaver's substrate.c and the original work by Jared Tarbell.
 * Distributed under the MIT License. See ../LICENSE.
 */

#ifndef SUBSTRATE_H
#define SUBSTRATE_H

#include <ScreenSaver.h>

#include <Point.h>

#include <random>
#include <vector>


class SubstrateScreenSaver : public BScreenSaver {
public:
	SubstrateScreenSaver(BMessage* archive, image_id image);

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool preview) override;
	void Draw(BView* view, int32 frame) override;

private:
	struct Crack {
		BPoint position;
		double angle;
		double angleIncrement;
		BPoint curveStep;
		double angleDrawn;
		rgb_color colour;
		double gain;
	};

	void _ApplySettings();
	void _ConfigureCanvas(BView* view);
	void _DrawSand(BView* view, Crack& crack);
	int16& _GridAt(int32 x, int32 y);
	void _ResetCrack(Crack& crack);
	void _Restart(BView* view);
	void _StepCracks(BView* view);
	uint32 _Random(uint32 upperBound);
	double _RandomUnit();

	std::vector<Crack> fCracks;
	std::vector<int16> fCrackGrid;
	std::mt19937 fRandomEngine;
	bigtime_t fLastRestartTime;
	int32 fWidth;
	int32 fHeight;
	int32 fNumberOfCracks;
	int32 fDuration;
	int32 fGrains;
	int32 fCirclePercentage;
	bool fDark;
	bool fWantRestart;
};


#endif
