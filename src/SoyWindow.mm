#include "SoyWindow.h"
#import <Cocoa/Cocoa.h>


NSCursor* CursorToNSCursor(SoyCursor::Type Cursor)
{
	switch ( Cursor )
	{
		default:
		case SoyCursor::Arrow:	return [NSCursor arrowCursor];
		case SoyCursor::Hand:	return [NSCursor openHandCursor];
		case SoyCursor::Busy:	return [NSCursor openHandCursor];
	}
}

void Platform::PushCursor(SoyCursor::Type Cursor)
{
	auto CursorNs = CursorToNSCursor( Cursor );
	[CursorNs push];
}

void Platform::PopCursor()
{
	[NSCursor pop];
}


void Platform::EnumScreens(std::function<void(TScreenMeta&)> EnumScreen)
{
	auto* Screens = [NSScreen screens];
	
	auto OnNsScreen = [&](NSScreen* Screen)
	{
		TScreenMeta ScreenMeta;
		
		//	visible is without title bar, so we return the display size
		//NSRect RectVisible = Screen.visibleFrame;
		NSRect Rect = Screen.frame;
		ScreenMeta.mRect.x = Rect.origin.x;
		ScreenMeta.mRect.y = Rect.origin.y;
		ScreenMeta.mRect.w = Rect.size.width;
		ScreenMeta.mRect.h = Rect.size.height;

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
}
