/*
 * Based on Space 1.2 by David Enderson.
 * Distributed under the GNU General Public License, as specified by the
 * original release.
 */

#ifndef SPACE_H
#define SPACE_H

#include <ScreenSaver.h>

#include <array>
#include <random>

class BView;


class SpaceScreenSaver : public BScreenSaver {
public:
	SpaceScreenSaver(BMessage* message, image_id image);

	status_t SaveState(BMessage* into) const override;
	void StartConfig(BView* view) override;
	status_t StartSaver(BView* view, bool) override;
	void Draw(BView* view, int32) override;

private:
	struct Star {
		double x;
		double y;
		double z;
	};

	static constexpr int32 kMaximumStars = 500;

	void _ApplySettings();
	void _ResetStar(Star& star, bool initialPosition);
	void _RestartStars();

	std::array<Star, kMaximumStars> fStars;
	std::minstd_rand fRandomEngine;
	bigtime_t fLastFrameTime;
	int32 fNumberOfStars;
	int32 fActiveStarCount;
	int32 fStarRadius;
	int32 fStarSpeed;
	int32 fStarAngle;
};


#endif
