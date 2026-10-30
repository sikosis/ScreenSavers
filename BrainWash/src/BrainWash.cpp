/*
 * BrainWash was created by Stephan Aßmus and Ingo Weinhold in 1998/99.
 * See ../COPYRIGHT for licensing information.
 */

#include "BrainWash.h"

#include "BrainWashConfigView.h"

#include <Bitmap.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cstddef>
#include <new>


namespace {

const char* const kCycleSpeedKey = "CYCLE_SPEED";
const char* const kDrawingGapKey = "DRAWING_GAP";
const char* const kDrawingSpeedKey = "DRAWING_SPEED";

const int32 kMinimumCycleSpeed = 1;
const int32 kMaximumCycleSpeed = 10;
const int32 kMinimumDrawingGap = 0;
const int32 kMaximumDrawingGap = 10;
const int32 kMinimumDrawingSpeed = 1;
const int32 kMaximumDrawingSpeed = 9;
const bigtime_t kFrameInterval = 33333;

}


extern "C" _EXPORT BScreenSaver*
instantiate_screen_saver(BMessage* archive, image_id image)
{
	return new(std::nothrow) BrainWashScreenSaver(archive, image);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BrainWashScreenSaver::BrainWashScreenSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fEngine(static_cast<uint32>(system_time())),
	fBitmap(nullptr),
	fPatternCompletedAt(0),
	fCycleSpeed(5),
	fDrawingGap(5),
	fDrawingSpeed(5)
{
	if (archive != nullptr) {
		archive->FindInt32(kCycleSpeedKey, &fCycleSpeed);
		archive->FindInt32(kDrawingGapKey, &fDrawingGap);
		archive->FindInt32(kDrawingSpeedKey, &fDrawingSpeed);
	}
	_ApplySettings();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BrainWashScreenSaver::~BrainWashScreenSaver()
{
	delete fBitmap;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BrainWashScreenSaver::SaveState(BMessage* into) const
{
	if (into == nullptr)
		return B_BAD_VALUE;

	status_t status = into->AddInt32(kCycleSpeedKey, fCycleSpeed);
	if (status == B_OK)
		status = into->AddInt32(kDrawingGapKey, fDrawingGap);
	if (status == B_OK)
		status = into->AddInt32(kDrawingSpeedKey, fDrawingSpeed);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashScreenSaver::StartConfig(BView* view)
{
	if (view == nullptr)
		return;
	view->AddChild(new BrainWashConfigView(view->Bounds(), &fCycleSpeed,
		&fDrawingGap, &fDrawingSpeed));
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BrainWashScreenSaver::StartSaver(BView* view, bool)
{
	if (view == nullptr)
		return B_BAD_VALUE;

	_ApplySettings();
	const status_t status = _ConfigureCanvas(view);
	if (status != B_OK)
		return status;

	view->SetViewColor(B_TRANSPARENT_COLOR);
	view->SetDrawingMode(B_OP_COPY);
	SetTickSize(kFrameInterval);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashScreenSaver::StopSaver()
{
	delete fBitmap;
	fBitmap = nullptr;
	fPatternCompletedAt = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashScreenSaver::Draw(BView* view, int32)
{
	if (view == nullptr)
		return;

	const int32 width = std::max(static_cast<int32>(view->Bounds().IntegerWidth()) + 1, 1);
	const int32 height = std::max(static_cast<int32>(view->Bounds().IntegerHeight()) + 1, 1);
	if (fBitmap == nullptr || width != fEngine.Width() || height != fEngine.Height()) {
		if (_ConfigureCanvas(view) != B_OK)
			return;
	}

	_ApplySettings();
	fEngine.AdvancePalette(fCycleSpeed);
	const bigtime_t now = system_time();
	if (fEngine.PatternComplete()) {
		if (fPatternCompletedAt == 0)
			fPatternCompletedAt = now;
		if (now - fPatternCompletedAt
			>= brainwash::DrawingGapMicroseconds(fDrawingGap)) {
			fEngine.BeginPattern();
			fPatternCompletedAt = 0;
		}
	} else if (fEngine.RenderLines(
		brainwash::DrawingLinesPerFrame(fDrawingSpeed))) {
		fPatternCompletedAt = now;
	}

	_RenderBitmap();
	view->DrawBitmap(fBitmap, fBitmap->Bounds(), view->Bounds());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashScreenSaver::_ApplySettings()
{
	fCycleSpeed = std::clamp(fCycleSpeed, kMinimumCycleSpeed, kMaximumCycleSpeed);
	fDrawingGap = std::clamp(fDrawingGap, kMinimumDrawingGap, kMaximumDrawingGap);
	fDrawingSpeed = std::clamp(fDrawingSpeed,
		kMinimumDrawingSpeed, kMaximumDrawingSpeed);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
BrainWashScreenSaver::_ConfigureCanvas(BView* view)
{
	const int32 width = std::max(static_cast<int32>(view->Bounds().IntegerWidth()) + 1, 1);
	const int32 height = std::max(static_cast<int32>(view->Bounds().IntegerHeight()) + 1, 1);
	BBitmap* bitmap = new(std::nothrow) BBitmap(BRect(0, 0, width - 1, height - 1),
		0, B_RGB32);
	if (bitmap == nullptr)
		return B_NO_MEMORY;
	const status_t status = bitmap->InitCheck();
	if (status != B_OK) {
		delete bitmap;
		return status;
	}

	delete fBitmap;
	fBitmap = bitmap;
	fEngine.Resize(width, height);
	fPatternCompletedAt = 0;
	_RenderBitmap();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
BrainWashScreenSaver::_RenderBitmap()
{
	if (fBitmap == nullptr)
		return;

	const std::vector<uint8_t>& pixels = fEngine.Pixels();
	const std::array<brainwash::Colour, 256>& palette = fEngine.Palette();
	uint8* bits = static_cast<uint8*>(fBitmap->Bits());
	const int32 bytesPerRow = fBitmap->BytesPerRow();
	for (int32 y = 0; y < fEngine.Height(); y++) {
		uint32* row = reinterpret_cast<uint32*>(bits + y * bytesPerRow);
		for (int32 x = 0; x < fEngine.Width(); x++) {
			const brainwash::Colour& colour
				= palette[pixels[static_cast<std::size_t>(y) * fEngine.Width() + x]];
			row[x] = 0xff000000 | static_cast<uint32>(colour.red) << 16
				| static_cast<uint32>(colour.green) << 8 | colour.blue;
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
