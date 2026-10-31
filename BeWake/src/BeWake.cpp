/*
 * BeWake
 * Copyright 2003 Mattia Tristo
 * Copyright 2009 Mattia Tristo and Gabriele Biffi
 *
 * Modified for current Haiku systems in 2026.
 * Distributed under the GNU General Public License, version 2.
 */

#include "BeWake.h"

#include "BeWakeConfigView.h"

#include <Bitmap.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <new>


namespace {

const char* const kCycleColoursKey = "CycleColors";
const char* const kRandomColourKey = "RandomColor";
const char* const kColourKey = "Color";
const bigtime_t kFrameInterval = 20000;

}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* archive, image_id image)
{
	return new(std::nothrow) BeWakeScreenSaver(archive, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BeWakeScreenSaver::BeWakeScreenSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fEngine(static_cast<uint32>(system_time())),
	fBackBitmap(nullptr),
	fBackView(nullptr),
	fColour(0.66f),
	fCycleColours(true),
	fRandomColour(false),
	fWantReset(false)
{
	if (archive != nullptr) {
		archive->FindBool(kCycleColoursKey, &fCycleColours);
		archive->FindBool(kRandomColourKey, &fRandomColour);
		archive->FindFloat(kColourKey, &fColour);
	}
	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BeWakeScreenSaver::~BeWakeScreenSaver()
{
	delete fBackBitmap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BeWakeScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddBool(kCycleColoursKey, fCycleColours);
	if (status == B_OK)
		status = into->AddBool(kRandomColourKey, fRandomColour);
	if (status == B_OK)
		status = into->AddFloat(kColourKey, fColour);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;
	view->AddChild(new BeWakeConfigView(view->Bounds(), &fColour,
		&fCycleColours, &fRandomColour, &fWantReset));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BeWakeScreenSaver::StartSaver(BView* view, bool)
{
	if (view == nullptr)
		return B_BAD_VALUE;
	_ApplySettings();
	if (!_EnsureBuffer(view->Bounds()))
		return B_NO_MEMORY;
	_SynchroniseEngine();
	view->SetDrawingMode(B_OP_COPY);
	SetTickSize(kFrameInterval);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeScreenSaver::StopSaver()
{
	delete fBackBitmap;
	fBackBitmap = nullptr;
	fBackView = nullptr;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr || !_EnsureBuffer(view->Bounds()))
		return;

	_ApplySettings();
	_SynchroniseEngine();
	if (fBackBitmap->Lock()) {
		fBackView->SetLowColor(0, 0, 0);
		fBackView->FillRect(fBackView->Bounds(), B_SOLID_LOW);
		fEngine.Step();

		const std::vector<bewake::WakeLine>& lines = fEngine.Lines();
		fBackView->BeginLineArray(static_cast<int32>(lines.size()));
		for (auto iterator = lines.rbegin(); iterator != lines.rend(); iterator++) {
			const bewake::Colour colour = bewake::HslToRgb(iterator->hue,
				iterator->saturation, iterator->lightness);
			fBackView->AddLine(BPoint(iterator->x1, iterator->y1),
				BPoint(iterator->x2, iterator->y2),
				{ colour.red, colour.green, colour.blue, colour.alpha });
		}
		fBackView->EndLineArray();
		fBackView->Sync();
		fBackBitmap->Unlock();
	}

	view->DrawBitmap(fBackBitmap, fBackBitmap->Bounds(), view->Bounds());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeScreenSaver::_ApplySettings()
{
	fColour = static_cast<float>(bewake::NormaliseHue(fColour));
	if (fCycleColours)
		fRandomColour = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
BeWakeScreenSaver::_EnsureBuffer(const BRect& bounds)
{
	const BRect bitmapBounds(0.0f, 0.0f, bounds.Width(), bounds.Height());
	if (fBackBitmap != nullptr && fBackBitmap->Bounds() == bitmapBounds)
		return true;

	BBitmap* bitmap = new(std::nothrow) BBitmap(bitmapBounds,
		B_BITMAP_ACCEPTS_VIEWS, B_RGBA32);
	if (bitmap == nullptr || bitmap->InitCheck() != B_OK) {
		delete bitmap;
		return false;
	}
	BView* bufferView = new(std::nothrow) BView(bitmapBounds, "BeWake buffer",
		B_FOLLOW_NONE, B_WILL_DRAW);
	if (bufferView == nullptr || !bitmap->Lock()) {
		delete bufferView;
		delete bitmap;
		return false;
	}
	bitmap->AddChild(bufferView);
	bufferView->SetViewColor(B_TRANSPARENT_COLOR);
	bufferView->SetDrawingMode(B_OP_COPY);
	bitmap->Unlock();

	delete fBackBitmap;
	fBackBitmap = bitmap;
	fBackView = bufferView;
	fEngine.Resize(std::max(static_cast<int32>(bitmapBounds.IntegerWidth()) + 1, 1),
		std::max(static_cast<int32>(bitmapBounds.IntegerHeight()) + 1, 1));
	fWantReset = false;
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BeWakeScreenSaver::_SynchroniseEngine()
{
	fEngine.SetFixedHue(fColour);
	const bewake::ColourMode mode = fCycleColours
		? bewake::ColourMode::Cycle
		: (fRandomColour ? bewake::ColourMode::Random : bewake::ColourMode::Fixed);
	if (mode != fEngine.CurrentColourMode())
		fEngine.SetColourMode(mode);
	else if (fWantReset)
		fEngine.Reset();
	fWantReset = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//
