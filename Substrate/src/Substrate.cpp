/*
 * Copyright (c) 2014 Josef Gajdusek
 * Based on XScreenSaver's substrate.c and the original work by Jared Tarbell.
 * Distributed under the MIT License. See ../LICENSE.
 */

#include "Substrate.h"

#include "SubstrateConfigView.h"
#include "SubstrateMath.h"

#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <new>


namespace {

const char* const kNumberOfCracksKey = "fNumCracks";
const char* const kDurationKey = "fDuration";
const char* const kGrainsKey = "fGrains";
const char* const kCirclePercentageKey = "fCirclePerc";
const char* const kDarkKey = "fDark";

const int32 kMinimumCracks = 1;
const int32 kMaximumCracks = 20;
const int32 kMinimumDuration = 3000;
const int32 kMaximumDuration = 10000;
const int32 kMinimumGrains = 0;
const int32 kMaximumGrains = 50;
const int32 kMinimumCirclePercentage = 0;
const int32 kMaximumCirclePercentage = 100;
const int16 kEmptyGridCell = 361;
const bigtime_t kFrameInterval = 16667;
const double kStep = 0.7;
const double kPi = 3.14159265358979323846;

const rgb_color kCrackColour = { 0, 0, 0, 255 };
const rgb_color kBackgroundColour = { 255, 253, 230, 255 };
const rgb_color kDarkCrackColour = { 255, 255, 255, 255 };
const rgb_color kDarkBackgroundColour = { 0, 2, 25, 255 };

const rgb_color kColourMap[] = {
	{ 0x20, 0x1f, 0x21, 0x0f }, { 0x35, 0x26, 0x26, 0x0f },
	{ 0x39, 0x2b, 0x2d, 0x0f }, { 0x47, 0x33, 0x29, 0x0f },
	{ 0x58, 0x40, 0x37, 0x0f }, { 0x65, 0x47, 0x2d, 0x0f },
	{ 0x6d, 0x5d, 0x3d, 0x0f }, { 0x74, 0x64, 0x33, 0x0f },
	{ 0x7c, 0x6c, 0x42, 0x0f }, { 0x6b, 0x73, 0x4b, 0x0f },
	{ 0x69, 0x6d, 0x5e, 0x0f }, { 0x57, 0x74, 0x6e, 0x0f },
	{ 0x9c, 0x54, 0x2b, 0x0f }, { 0xaa, 0x73, 0x30, 0x0f },
	{ 0xc4, 0x5a, 0x27, 0x0f }, { 0xd9, 0x52, 0x23, 0x0f },
	{ 0xe5, 0x70, 0x37, 0x0f }, { 0x93, 0x73, 0x52, 0x0f },
	{ 0x81, 0x7b, 0x63, 0x0f }, { 0xd9, 0x89, 0x3b, 0x0f },
	{ 0xe4, 0x98, 0x32, 0x0f }, { 0xf0, 0xab, 0x3b, 0x0f },
	{ 0xb2, 0x9a, 0x58, 0x0f }, { 0x90, 0x9a, 0x7a, 0x0f },
	{ 0xa9, 0x99, 0x68, 0x0f }, { 0xeb, 0x8d, 0x43, 0x0f },
	{ 0xc2, 0x91, 0x68, 0x0f }, { 0xd1, 0xa9, 0x77, 0x0f },
	{ 0xf0, 0xe2, 0x7b, 0x0f }, { 0xc0, 0xb9, 0x99, 0x0f },
	{ 0xe6, 0xb8, 0x8f, 0x0f }, { 0xe0, 0xc8, 0x86, 0x0f },
	{ 0xf5, 0xda, 0x83, 0x0f }, { 0xec, 0xde, 0x9d, 0x0f },
	{ 0xf4, 0xe1, 0x93, 0x0f }, { 0xe7, 0xd8, 0xa7, 0x0f },
	{ 0xf4, 0xdb, 0xb5, 0x0f }, { 0xf5, 0xe9, 0xad, 0x0f },
	{ 0xf4, 0xe3, 0xbe, 0x0f }, { 0xf7, 0xf0, 0xb6, 0x0f },
	{ 0xd9, 0xd1, 0xc1, 0x0f }, { 0xf3, 0xed, 0xc7, 0x0f }
};


rgb_color
InvertColour(rgb_color colour)
{
	colour.red = 255 - colour.red;
	colour.green = 255 - colour.green;
	colour.blue = 255 - colour.blue;
	return colour;
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
Radians(double degrees)
{
	return degrees * kPi / 180.0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* archive, image_id image)
{
	return new(std::nothrow) SubstrateScreenSaver(archive, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


SubstrateScreenSaver::SubstrateScreenSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fRandomEngine(static_cast<uint32>(system_time())),
	fLastRestartTime(0),
	fWidth(0),
	fHeight(0),
	fNumberOfCracks(5),
	fDuration(5000),
	fGrains(50),
	fCirclePercentage(30),
	fDark(false),
	fWantRestart(true)
{
	if (archive != nullptr) {
		archive->FindInt32(kNumberOfCracksKey, &fNumberOfCracks);
		archive->FindInt32(kDurationKey, &fDuration);
		archive->FindInt32(kGrainsKey, &fGrains);
		archive->FindInt32(kCirclePercentageKey, &fCirclePercentage);
		archive->FindBool(kDarkKey, &fDark);
	}

	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SubstrateScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddInt32(kNumberOfCracksKey, fNumberOfCracks);
	if (status == B_OK)
		status = into->AddInt32(kDurationKey, fDuration);
	if (status == B_OK)
		status = into->AddInt32(kGrainsKey, fGrains);
	if (status == B_OK)
		status = into->AddInt32(kCirclePercentageKey, fCirclePercentage);
	if (status == B_OK)
		status = into->AddBool(kDarkKey, fDark);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;

	view->AddChild(new SubstrateConfigView(view->Bounds(), &fNumberOfCracks,
		&fDuration, &fGrains, &fCirclePercentage, &fDark, &fWantRestart));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SubstrateScreenSaver::StartSaver(BView* view, bool)
{
	if (view == nullptr)
		return B_BAD_VALUE;

	_ApplySettings();
	_ConfigureCanvas(view);
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
	view->SetFlags(view->Flags() | B_SUBPIXEL_PRECISE);
	SetTickSize(kFrameInterval);
	fWantRestart = true;
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	const int32 width = std::max(static_cast<int32>(view->Bounds().IntegerWidth()) + 1, 1);
	const int32 height = std::max(static_cast<int32>(view->Bounds().IntegerHeight()) + 1, 1);
	if (width != fWidth || height != fHeight) {
		_ConfigureCanvas(view);
		fWantRestart = true;
	}

	_ApplySettings();
	const bigtime_t now = system_time();
	if (fWantRestart
		|| now - fLastRestartTime >= substrate::DurationMicroseconds(fDuration)) {
		_Restart(view);
	}
	_StepCracks(view);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_ApplySettings()
{
	fNumberOfCracks = std::clamp(fNumberOfCracks, kMinimumCracks, kMaximumCracks);
	fDuration = std::clamp(fDuration, kMinimumDuration, kMaximumDuration);
	fGrains = std::clamp(fGrains, kMinimumGrains, kMaximumGrains);
	fCirclePercentage = std::clamp(fCirclePercentage,
		kMinimumCirclePercentage, kMaximumCirclePercentage);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_ConfigureCanvas(BView* view)
{
	fWidth = std::max(static_cast<int32>(view->Bounds().IntegerWidth()) + 1, 1);
	fHeight = std::max(static_cast<int32>(view->Bounds().IntegerHeight()) + 1, 1);
	fCrackGrid.assign(static_cast<std::size_t>(fWidth) * fHeight, kEmptyGridCell);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_Restart(BView* view)
{
	view->SetHighColor(fDark ? kDarkBackgroundColour : kBackgroundColour);
	view->FillRect(view->Bounds());

	std::fill(fCrackGrid.begin(), fCrackGrid.end(), kEmptyGridCell);
	fCracks.resize(fNumberOfCracks);
	for (Crack& crack : fCracks)
		_ResetCrack(crack);

	fLastRestartTime = system_time();
	fWantRestart = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_ResetCrack(Crack& crack)
{
	int32 x = 0;
	int32 y = 0;
	bool foundExistingCrack = false;
	for (int32 attempt = 0; attempt < 10000; attempt++) {
		x = static_cast<int32>(_Random(fWidth));
		y = static_cast<int32>(_Random(fHeight));
		if (_GridAt(x, y) <= 360) {
			foundExistingCrack = true;
			break;
		}
	}

	if (foundExistingCrack) {
		crack.angle = _GridAt(x, y) + (_Random(2) == 0 ? -90.0 : 90.0);
	} else {
		crack.angle = _Random(360);
	}

	crack.position.Set(x, y);
	crack.colour = kColourMap[_Random(
		static_cast<uint32>(sizeof(kColourMap) / sizeof(kColourMap[0])))];
	if (fDark)
		crack.colour = InvertColour(crack.colour);
	crack.gain = _RandomUnit();
	crack.angleDrawn = 0.0;

	if (_Random(100) < static_cast<uint32>(fCirclePercentage)) {
		const int32 maximumRadius = std::max((fWidth + fHeight) / 2, 10);
		double radius = 10.0 + _Random(static_cast<uint32>(maximumRadius - 9));
		if (_Random(2) == 0)
			radius = -radius;
		const double angleIncrement = kStep / radius;
		crack.angleIncrement = angleIncrement * 180.0 / kPi;
		crack.curveStep.x = radius * (1.0 - std::cos(angleIncrement));
		crack.curveStep.y = radius * std::sin(angleIncrement);
	} else {
		crack.angleIncrement = 0.0;
		crack.curveStep.Set(0.0, 0.0);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_StepCracks(BView* view)
{
	view->SetPenSize(2.0);
	for (Crack& crack : fCracks) {
		const BPoint previousPosition = crack.position;
		const double angle = Radians(crack.angle);

		if (crack.angleIncrement != 0.0) {
			crack.position.x += crack.curveStep.y * std::cos(angle);
			crack.position.y += crack.curveStep.y * std::sin(angle);
			crack.position.x += crack.curveStep.x * std::cos(angle - kPi / 2.0);
			crack.position.y += crack.curveStep.x * std::sin(angle - kPi / 2.0);
			crack.angle += crack.angleIncrement;
			crack.angleDrawn += std::abs(crack.angleIncrement);
		} else {
			crack.position.x += kStep * std::cos(angle);
			crack.position.y += kStep * std::sin(angle);
		}

		const int32 x = static_cast<int32>(crack.position.x);
		const int32 y = static_cast<int32>(crack.position.y);
		bool finished = x < 0 || x >= fWidth || y < 0 || y >= fHeight;
		if (!finished && _GridAt(x, y) <= 360
			&& substrate::AngularDifference(_GridAt(x, y), crack.angle) > 5.0) {
			finished = true;
		}
		if (crack.angleIncrement != 0.0 && crack.angleDrawn > 360.0)
			finished = true;

		if (finished) {
			_ResetCrack(crack);
			continue;
		}

		_GridAt(x, y) = static_cast<int16>(substrate::NormaliseAngle(crack.angle));
		_DrawSand(view, crack);
		view->BeginLineArray(1);
		view->AddLine(previousPosition, crack.position,
			fDark ? kDarkCrackColour : kCrackColour);
		view->EndLineArray();
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SubstrateScreenSaver::_DrawSand(BView* view, Crack& crack)
{
	if (fGrains <= 0)
		return;

	const double angle = Radians(crack.angle);
	BPoint edge(crack.position);
	edge.x += 4.0 * std::sin(angle);
	edge.y -= 4.0 * std::cos(angle);

	const int32 maximumSteps = (fWidth + fHeight) * 2;
	for (int32 step = 0; step < maximumSteps; step++) {
		edge.x += 0.81 * std::sin(angle);
		edge.y -= 0.81 * std::cos(angle);
		const int32 x = static_cast<int32>(edge.x);
		const int32 y = static_cast<int32>(edge.y);
		if (x < 0 || x >= fWidth || y < 0 || y >= fHeight
			|| _GridAt(x, y) <= 360) {
			break;
		}
	}

	crack.gain = std::clamp(crack.gain + (_RandomUnit() - 0.5) * 0.2, 0.0, 1.0);
	view->SetHighColor(crack.colour);
	for (int32 index = 0; index < fGrains; index++) {
		const double factor = substrate::SandPositionFactor(index, fGrains,
			crack.gain);
		const int32 x = static_cast<int32>(crack.position.x
			+ (edge.x - crack.position.x) * factor);
		const int32 y = static_cast<int32>(crack.position.y
			+ (edge.y - crack.position.y) * factor);
		view->FillRect(BRect(x, y, x + 1, y + 1));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


int16&
SubstrateScreenSaver::_GridAt(int32 x, int32 y)
{
	return fCrackGrid[static_cast<std::size_t>(y) * fWidth + x];
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint32
SubstrateScreenSaver::_Random(uint32 upperBound)
{
	if (upperBound == 0)
		return 0;
	std::uniform_int_distribution<uint32> distribution(0, upperBound - 1);
	return distribution(fRandomEngine);
}
//---------------------------------------------------------------------------------------------------------------------------------//


double
SubstrateScreenSaver::_RandomUnit()
{
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	return distribution(fRandomEngine);
}
//---------------------------------------------------------------------------------------------------------------------------------//

