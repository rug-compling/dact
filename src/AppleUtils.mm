#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>

#include "AppleUtils.hh"

void enableFullScreenOnMac(QMainWindow const *window)
{
	NSView *nsview = (NSView *) window->winId();
	NSWindow *nswindow = [nsview window];
	[nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
}

void toggleFullScreenOnMac(QMainWindow const *window)
{
	NSView *nsview = (NSView *) window->winId();
	NSWindow *nswindow = [nsview window];
	[nswindow toggleFullScreen:nil];
}
