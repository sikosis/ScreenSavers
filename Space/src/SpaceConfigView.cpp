/*
 * Based on Space 1.2 by David Enderson.
 * Distributed under the GNU General Public License, as specified by the
 * original release.
 */

#include "SpaceConfigView.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <Slider.h>
#include <StringView.h>


namespace {

enum {
	kAngleChanged = 'angl',
	kNumberChanged = 'nums',
	kRadiusChanged = 'diam',
	kSpeedChanged = 'sped'
};

}


SpaceConfigView::SpaceConfigView(BRect frame, int32* numberOfStars, int32* starRadius,
	int32* starSpeed, int32* starAngle)
	:
	BView(frame, "Space settings", B_FOLLOW_ALL, B_WILL_DRAW),
	fAngleSlider(new BSlider("angle", "Direction", new BMessage(kAngleChanged),
		0, 628, B_HORIZONTAL, B_BLOCK_THUMB)),
	fNumberSlider(new BSlider("number", "Number of stars",
		new BMessage(kNumberChanged), 1, 500, B_HORIZONTAL, B_BLOCK_THUMB)),
	fRadiusSlider(new BSlider("radius", "Star diameter",
		new BMessage(kRadiusChanged), 1, 18, B_HORIZONTAL, B_BLOCK_THUMB)),
	fSpeedSlider(new BSlider("speed", "Warp speed",
		new BMessage(kSpeedChanged), 1, 850, B_HORIZONTAL, B_BLOCK_THUMB)),
	fNumberOfStars(numberOfStars),
	fStarRadius(starRadius),
	fStarSpeed(starSpeed),
	fStarAngle(starAngle)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* title = new BStringView("title", "Space");
	title->SetFont(be_bold_font);
	BStringView* description = new BStringView("description",
		"A perspective star field by David Enderson");

	fAngleSlider->SetLimitLabels("0\xC2\xB0", "360\xC2\xB0");
	fAngleSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fAngleSlider->SetHashMarkCount(9);
	fAngleSlider->SetValue(*fStarAngle);

	fNumberSlider->SetLimitLabels("1", "500");
	fNumberSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fNumberSlider->SetHashMarkCount(11);
	fNumberSlider->SetValue(*fNumberOfStars);

	fRadiusSlider->SetLimitLabels("Tiny", "Huge");
	fRadiusSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fRadiusSlider->SetHashMarkCount(10);
	fRadiusSlider->SetValue(*fStarRadius);

	fSpeedSlider->SetLimitLabels("Drift", "Warp");
	fSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fSpeedSlider->SetHashMarkCount(11);
	fSpeedSlider->SetValue(*fStarSpeed);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(title)
		.Add(description)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fAngleSlider)
		.Add(fNumberSlider)
		.Add(fRadiusSlider)
		.Add(fSpeedSlider)
		.AddGlue();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceConfigView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fAngleSlider->SetTarget(this);
	fNumberSlider->SetTarget(this);
	fRadiusSlider->SetTarget(this);
	fSpeedSlider->SetTarget(this);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kAngleChanged:
			*fStarAngle = fAngleSlider->Value();
			break;
		case kNumberChanged:
			*fNumberOfStars = fNumberSlider->Value();
			break;
		case kRadiusChanged:
			*fStarRadius = fRadiusSlider->Value();
			break;
		case kSpeedChanged:
			*fStarSpeed = fSpeedSlider->Value();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

