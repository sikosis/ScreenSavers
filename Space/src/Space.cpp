/*
 * Space creates a three-dimensional field of stars and moves the viewer
 * through it. The perspective model is based on David Enderson's Space 1.2.
 *
 * Distributed under the GNU General Public License, as specified by the
 * original release.
 */

#include "Space.h"

#include "SpaceConfigView.h"

#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <new>


namespace {

const char* const kNumberOfStarsKey = "nofstars";
const char* const kStarRadiusKey = "rofstars";
const char* const kStarSpeedKey = "sofstars";
const char* const kStarAngleKey = "aofstars";

const bigtime_t kFrameInterval = 16667;
const bigtime_t kMaximumFrameTime = 100000;
const double kHorizontalFieldOfView = 0.4636;
const double kVerticalFieldOfView = 0.3588;
const double kNearPlane = 1.0;
const double kPi = 3.14159265358979323846;

}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* message, image_id image)
{
	return new(std::nothrow) SpaceScreenSaver(message, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


SpaceScreenSaver::SpaceScreenSaver(BMessage* message, image_id image)
	:
	BScreenSaver(message, image),
	fRandomEngine(static_cast<uint32>(system_time())),
	fLastFrameTime(0),
	fNumberOfStars(150),
	fActiveStarCount(0),
	fStarRadius(5),
	fStarSpeed(50),
	fStarAngle(0)
{
	if (message != nullptr) {
		message->FindInt32(kNumberOfStarsKey, &fNumberOfStars);
		message->FindInt32(kStarRadiusKey, &fStarRadius);
		message->FindInt32(kStarSpeedKey, &fStarSpeed);
		message->FindInt32(kStarAngleKey, &fStarAngle);
	}

	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SpaceScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddInt32(kNumberOfStarsKey, fNumberOfStars);
	if (status == B_OK)
		status = into->AddInt32(kStarRadiusKey, fStarRadius);
	if (status == B_OK)
		status = into->AddInt32(kStarSpeedKey, fStarSpeed);
	if (status == B_OK)
		status = into->AddInt32(kStarAngleKey, fStarAngle);

	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;

	view->AddChild(new SpaceConfigView(view->Bounds(), &fNumberOfStars, &fStarRadius,
		&fStarSpeed, &fStarAngle));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SpaceScreenSaver::StartSaver(BView* view, bool)
{
	if (view == nullptr)
		return B_BAD_VALUE;

	_ApplySettings();
	_RestartStars();
	fLastFrameTime = system_time();

	view->SetViewColor(B_TRANSPARENT_COLOR);
	view->SetLowColor(0, 0, 0);
	view->SetHighColor(255, 255, 255);
	view->FillRect(view->Bounds(), B_SOLID_LOW);

	SetTickSize(kFrameInterval);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	const int32 previousStarCount = fActiveStarCount;
	_ApplySettings();
	if (fNumberOfStars > previousStarCount) {
		for (int32 index = previousStarCount; index < fNumberOfStars; index++)
			_ResetStar(fStars[index], true);
	}
	fActiveStarCount = fNumberOfStars;

	const bigtime_t now = system_time();
	bigtime_t elapsed = now - fLastFrameTime;
	if (elapsed < 0)
		elapsed = 0;
	else if (elapsed > kMaximumFrameTime)
		elapsed = kMaximumFrameTime;
	fLastFrameTime = now;

	const BRect bounds = view->Bounds();
	const double halfWidth = bounds.Width() / 2.0;
	const double halfHeight = bounds.Height() / 2.0;
	if (halfWidth <= 0.0 || halfHeight <= 0.0)
		return;

	view->SetLowColor(0, 0, 0);
	view->FillRect(bounds, B_SOLID_LOW);

	const double distanceTravelled
		= static_cast<double>(elapsed) * fStarSpeed / 20000.0;
	const double angle = fStarAngle / 100.0;
	const double sine = std::sin(angle);
	const double cosine = std::cos(angle);

	for (int32 index = 0; index < fActiveStarCount; index++) {
		Star& star = fStars[index];
		star.z -= distanceTravelled;
		if (star.z < -15000.0)
			_ResetStar(star, false);

		const double rotatedX = star.x * cosine - star.z * sine;
		const double rotatedZ = star.x * sine + star.z * cosine + 15000.0;
		if (rotatedZ <= kNearPlane)
			continue;

		const double sightDistance = std::sqrt(rotatedX * rotatedX
			+ star.y * star.y + rotatedZ * rotatedZ);
		const double radius = std::atan2(fStarRadius, sightDistance)
			/ (kHorizontalFieldOfView / halfWidth);
		const double x = bounds.left + (1.0
			+ std::atan2(rotatedX, rotatedZ) / kHorizontalFieldOfView)
			* halfWidth;
		const double y = bounds.top + (1.0
			+ std::atan2(star.y, rotatedZ) / kVerticalFieldOfView)
			* halfHeight;

		if (!std::isfinite(radius) || !std::isfinite(x) || !std::isfinite(y)
			|| x + radius < bounds.left || x - radius > bounds.right
			|| y + radius < bounds.top || y - radius > bounds.bottom) {
			continue;
		}

		int32 brightness = 255;
		if (sightDistance > 20000.0) {
			brightness = static_cast<int32>((33000.0 - sightDistance) / 51.0);
			brightness = std::clamp(brightness, 0, 255);
		}
		view->SetHighColor(brightness, brightness, brightness);

		if (radius < 1.0) {
			const float pixelX = std::floor(x + 0.5);
			const float pixelY = std::floor(y + 0.5);
			view->FillRect(BRect(pixelX, pixelY, pixelX, pixelY), B_SOLID_HIGH);
		} else {
			view->FillEllipse(BPoint(x, y), radius, radius, B_SOLID_HIGH);
		}
	}

	view->SetHighColor(255, 255, 255);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceScreenSaver::_ApplySettings()
{
	fNumberOfStars = std::clamp(fNumberOfStars, 1, kMaximumStars);
	fStarRadius = std::clamp(fStarRadius, 1, 18);
	fStarSpeed = std::clamp(fStarSpeed, 1, 850);
	fStarAngle = std::clamp(fStarAngle, 0,
		static_cast<int32>(std::round(2.0 * kPi * 100.0)));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceScreenSaver::_ResetStar(Star& star, bool initialPosition)
{
	std::uniform_real_distribution<double> horizontal(-16500.0, 16500.0);
	std::uniform_real_distribution<double> vertical(-12375.0, 12375.0);
	std::uniform_real_distribution<double> initialDepth(-15000.0, 15000.0);
	std::uniform_real_distribution<double> farDepth(15000.0, 18000.0);

	star.x = horizontal(fRandomEngine);
	star.y = vertical(fRandomEngine);
	star.z = initialPosition ? initialDepth(fRandomEngine) : farDepth(fRandomEngine);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SpaceScreenSaver::_RestartStars()
{
	for (int32 index = 0; index < fNumberOfStars; index++)
		_ResetStar(fStars[index], true);

	fActiveStarCount = fNumberOfStars;
}
//---------------------------------------------------------------------------------------------------------------------------------//

