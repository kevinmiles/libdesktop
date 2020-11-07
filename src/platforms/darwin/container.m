
#include "libdesktop.h"
#import <Cocoa/Cocoa.h>
#include <yoga/Yoga.h>

@interface DskContainer : NSView
	@property napi_ref wrapper;
	@property YGNodeRef yoganode;
	@property(readonly) bool isFlipped;
@end

@implementation DskContainer
- (bool)isFlipped {
	return true;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}
@end

void dsk_widget_reposition(napi_env env, UIHandle container, UIHandle widget, float xcoord,
						   float ycoord, float width, float height) {
	NSView *view = (NSView *)widget;

	//NSRect parent = [(NSView *)container frame];

	//xcoord -= parent.origin.x;
	//ycoord -= parent.origin.y;

	printf("MOVE %.0f:%.0f %.0fx%.0f\n",xcoord,ycoord,width,height);

	[view setFrame:NSMakeRect(xcoord, ycoord, width, height)];
}

void dsk_platform_container_add_child(UIHandle parent, UIHandle child) {
	NSView *cnt = parent;
	// printf("\n\nCHILD: %p\n\n");

	[cnt addSubview:(NSView *)child];
}

DSK_EXTEND_MODULE(libdesktop);

DSK_DEFINE_CLASS(libdesktop, Container) {
	DSK_JS_FUNC_INIT();
	DSK_EXACTLY_NARGS(2);


	DskContainer *widget = [[DskContainer alloc] init];
	// widget.translatesAutoresizingMaskIntoConstraints = true;
	// widget.frame = NSMakeRect(0,0,800,600);
	//[widget setHidden:NO];
	dsk_wrap_widget(env, widget, this);
	if (dsk_set_properties(env, argv[0], this)) {
		napi_throw_error(env, NULL, "Error while setting widget properties.\n");
		return NULL;
	}
	dsk_append_all_children(env, widget, argv[1]);

	return this;
}
