
#include "libdesktop-host.h"
#include "libdesktop.h"
#include "yoga.h"
#include <WinUser.h>
#include <stdio.h>
#include <windows.h>
//#include <uv.h>

// ui internal window messages
// TODO make these either not messages or WM_USER-based, so we can be sane about reserving WM_APP
/*enum {
	// redirected WM_COMMAND and WM_NOTIFY
	msgCOMMAND = WM_APP + 0x40,		// start offset just to be safe
	msgNOTIFY,
	msgHSCROLL,
	msgQueued,
	msgD2DScratchPaint,
	msgD2DScratchLButtonDown,
};*/

void uiQueueMain(void (*f)(void *data), void *data);
ATOM registerWindowClass(HICON hDefaultIcon, HCURSOR hDefaultCursor);
ATOM initContainer();
HWND dummy;
#define windowClass L"libui_uiWindowClass"

const char *dsk_init() {
	if (registerWindowClass(NULL, NULL) == 0) {
		printf("error registering uiWindow window class\n");
	}
	initContainer();
	HINSTANCE hInstance = GetModuleHandle(NULL);
	dummy = CreateWindowExW(0, windowClass, L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
							0, 0, NULL, NULL, hInstance, NULL);

	return NULL;
}
void noop(void *data) {}

enum {
	// redirected WM_COMMAND and WM_NOTIFY
	msgCOMMAND = WM_APP + 0x40,		// start offset just to be safe
	msgQueued,
};

int dsk_wakeup_ui_loop() {
	PostMessageW(dummy, msgQueued, 0,0);
	//uiQueueMain(noop, NULL);
	return 0;
}


void uiMainSteps(void) {
	// don't need to do anything here
}

static int peekMessage(MSG *msg) {
	BOOL res;

	res = PeekMessageW(msg, NULL, 0, 0, PM_REMOVE);
	if (res == 0)
		return 2; // no message available
	if (msg->message != WM_QUIT)
		return 1; // a message
	return 0;	 // WM_QUIT
}

HWND parentToplevel(HWND child) {
	return GetAncestor(child, GA_ROOT);
}

static void processMessage(MSG *msg) {
	HWND correctParent;

	if (msg->hwnd != NULL)
		correctParent = parentToplevel(msg->hwnd);
	else // just to be safe
		correctParent = GetActiveWindow();
	if (correctParent != NULL)
		// this calls our mesage filter above for us
		if (IsDialogMessage(correctParent, msg) != 0)
			return;
	TranslateMessage(msg);
	DispatchMessageW(msg);
}

static int waitMessage(MSG *msg) {
	int res;

	res = GetMessageW(msg, NULL, 0, 0);
	if (res < 0) {
		// printf("error calling GetMessage()\n");
		return 0; // bail out on error
	}
	return res != 0; // returns false on WM_QUIT
}

int dsk_process_ui_event(int wait) {
	MSG msg;

	if (wait) {
		if (!waitMessage(&msg))
			return 0;
		processMessage(&msg);
		return 1;
	}

	// don't wait for a message
	switch (peekMessage(&msg)) {
	case 0: // quit
		// TODO PostQuitMessage() again?
		return 0;
	case 1: // process a message
		processMessage(&msg);
		// fall out to the case for no message
	}
	return 1; // no message
}

void dsk_quit(void) {
	PostQuitMessage(0);
}


int dsk_ui_events_pending() {
	MSG msg;
	return PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
}

struct _internal_uv_loop_s {
	/* User data - use this for whatever. */
	void *data;
	/* Loop reference counting. */
	unsigned int active_handles;
	void *handle_queue[2];
	void *active_reqs[2];
	/* Internal flag to signal loop stop. */
	unsigned int stop_flag;
	/* The loop's I/O completion port */
	void *iocp;
	/* The current time according to the event loop. in msecs.
	//  uint64_t tim
	// Tail of a single-linked circular queue of pending reqs. If the queue
	// is empty, tail_ is NULL. If there is only one item,
	// tail_->next_req == tail_
	void* pending_reqs_tail;
	// Head of a single-linked list of closed handles
	void* endgame_handles;
	// The head of the timers tree
	struct uv_timer_tree_s timers;
	// Lists of active loop (prepare / check / idle) watchers
	uv_prepare_t* prepare_handles;
	uv_check_t* check_handles;
	uv_idle_t* idle_handles;
	// This pointer will refer to the prepare/check/idle handle whose
	// callback is scheduled to be called next. This is needed to allow
	// safe removal from one of the lists above while that list being
	// iterated over.
	uv_prepare_t* next_prepare_handle;
	uv_check_t* next_check_handle;
	uv_idle_t* next_idle_handle;
	// This handle holds the peer sockets for the fast variant of uv_poll_t
	SOCKET poll_peer_sockets[UV_MSAFD_PROVIDER_COUNT];
	// Counter to keep track of active tcp streams
	unsigned int active_tcp_streams;
	// Counter to keep track of active udp streams
	unsigned int active_udp_streams;
	// Counter to started timer
	uint64_t timer_counter;
	// Threadpool
	void* wq[2];
	uv_mutex_t wq_mutex;
	uv_async_t wq_async;
	*/
};

int dsk_wait_node_events(uv_loop_t *loop, int timeout) {
	DWORD bytes;
	ULONG_PTR key;
	OVERLAPPED *overlapped;

	struct _internal_uv_loop_s *_loop = (struct _internal_uv_loop_s *)loop;

	if (timeout == -1) {
		timeout = INFINITE;
	}

	int ret = GetQueuedCompletionStatus(_loop->iocp, &bytes, &key, &overlapped, timeout);

	// Does we need to requeue the queued completions?
	// this happen to be same code used by Electron
	// on Windows:
	// https://github.com/electron/electron/blob/master/atom/common/node_bindings_win.cc#L24
	// but the application become unstable when this
	// part is uncommented.
	// See also this PRs on libuv repo:
	// https://github.com/libuv/libuv/pull/1007
	// https://github.com/libuv/libuv/pull/1544
	// https://github.com/libuv/libuv/pull/1651
	if (overlapped != NULL) {
		PostQueuedCompletionStatus(_loop->iocp, bytes, key, overlapped);
	}

	return ret;
}


void dsk_connect_event(UIHandle widget, char *eventname, struct dsk_event_args *args) {
	// g_signal_connect(G_OBJECT(widget), eventname, G_CALLBACK(dsk_on_event), args);
}
