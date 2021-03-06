#include "SoyWindow.h"

#if defined(TARGET_OSX)
#import <Cocoa/Cocoa.h>
#endif

#if defined(TARGET_OSX)
NSCursor* CursorToNSCursor(SoyCursor::Type Cursor)
{
	switch ( Cursor )
	{
			//	unused;
			//	openHandCursor
			//	closedHandCursor
			//	resizeLeftCursor
			//	resizeRightCursor
			//	resizeDownCursor
			//	disappearingItemCursor
			
		default:
		case SoyCursor::Arrow:			return [NSCursor arrowCursor];
		case SoyCursor::Hand:			return [NSCursor pointingHandCursor];
		case SoyCursor::ResizeVert:		return [NSCursor resizeUpDownCursor];
		case SoyCursor::ResizeHorz:		return [NSCursor resizeLeftRightCursor];
		case SoyCursor::UpArrow:		return [NSCursor resizeUpCursor];
		case SoyCursor::TextCursor:		return [NSCursor IBeamCursor];
		case SoyCursor::Cross:			return [NSCursor crosshairCursor];
		case SoyCursor::NotAllowed:		return [NSCursor operationNotAllowedCursor];

		//	osx doesn't have these, we should implement our own
		case SoyCursor::ArrowAndWait:
		case SoyCursor::Wait:
			return [NSCursor arrowCursor];
	
		//	wrong for now
		case SoyCursor::ResizeAll:			return [NSCursor dragCopyCursor];
		case SoyCursor::ResizeNorthEast:	return [NSCursor dragCopyCursor];
		case SoyCursor::ResizeNorthWest:	return [NSCursor dragCopyCursor];
		case SoyCursor::Help:			return [NSCursor contextualMenuCursor];
	}
}
#endif

#if defined(TARGET_OSX)
void Platform::PushCursor(SoyCursor::Type Cursor)
{
	auto CursorNs = CursorToNSCursor( Cursor );
	[CursorNs push];
}
#endif

#if defined(TARGET_OSX)
void Platform::PopCursor()
{
	[NSCursor pop];
}
#endif

void Platform::EnumScreens(std::function<void(TScreenMeta&)> EnumScreen)
{
#if defined(TARGET_OSX)
	auto* Screens = [NSScreen screens];
	
	auto OnNsScreen = [&](NSScreen* Screen)
	{
		TScreenMeta ScreenMeta;
		
		//	visible is without title bar, so we return the display size
		NSRect VisibleRect = Screen.visibleFrame;
		NSRect FullRect = Screen.frame;
		ScreenMeta.mFullRect.x = FullRect.origin.x;
		ScreenMeta.mFullRect.y = FullRect.origin.y;
		ScreenMeta.mFullRect.w = FullRect.size.width;
		ScreenMeta.mFullRect.h = FullRect.size.height;
		
		ScreenMeta.mWorkRect.x = VisibleRect.origin.x;
		ScreenMeta.mWorkRect.y = VisibleRect.origin.y;
		ScreenMeta.mWorkRect.w = VisibleRect.size.width;
		ScreenMeta.mWorkRect.h = VisibleRect.size.height;

		//	get unique identifier
		//	https://stackoverflow.com/a/16164331/355753
		auto DeviceMeta = Screen.deviceDescription;
		auto ScreenNumber = [[DeviceMeta objectForKey:@"NSScreenNumber"] integerValue];
		std::stringstream Name;
		Name << ScreenNumber;
		ScreenMeta.mName = Name.str();
		
		EnumScreen( ScreenMeta );
	};
	Platform::NSArray_ForEach<NSScreen*>( Screens, OnNsScreen );
#endif
}
