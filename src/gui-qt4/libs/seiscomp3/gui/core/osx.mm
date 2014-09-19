/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/gui/core/osx.h>
#include <QMainWindow>
#import <Cocoa/Cocoa.h>

namespace Seiscomp {
namespace Gui {
namespace Mac {


bool isLion() {
	NSString *string = [NSString string];
	// this selector was added only in Lion. so we can check if it's responding, we are on Lion
	return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}


bool addFullscreen(QMainWindow *w) {
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
	if ( isLion() ) { // checks if lion is running
		NSView *nsview = (NSView *)w->winId();
		NSWindow *nswindow = [nsview window];
		[nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		return true;
	}
#endif
	return false;
}


}
}
}
