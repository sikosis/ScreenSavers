/*
 * Based on Space 1.2 by David Enderson.
 * Distributed under the GNU General Public License, as specified by the
 * original release.
 */

#ifndef SPACE_CONFIG_VIEW_H
#define SPACE_CONFIG_VIEW_H

#include <View.h>

class BSlider;


class SpaceConfigView : public BView {
public:
	SpaceConfigView(BRect frame, int32* numberOfStars, int32* starRadius,
		int32* starSpeed, int32* starAngle);

	void AttachedToWindow() override;
	void MessageReceived(BMessage* message) override;

private:
	BSlider* fAngleSlider;
	BSlider* fNumberSlider;
	BSlider* fRadiusSlider;
	BSlider* fSpeedSlider;
	int32* fNumberOfStars;
	int32* fStarRadius;
	int32* fStarSpeed;
	int32* fStarAngle;
};


#endif
