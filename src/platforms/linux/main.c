
#include <gtk/gtk.h>
#include <math.h>
#include <dlfcn.h>		// see future.c
#include <langinfo.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/epoll.h>
#include <uv.h>

const char *uiInit()
{
	GError *err = NULL;
	const char *msg;

	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &err) == FALSE) {
		msg = g_strdup(err->message);
		g_error_free(err);
		return msg;
	}
	return NULL;
}

void uiFreeInitError(const char *err)
{
	g_free((gpointer) err);
}

static gboolean (*iteration)(gboolean) = NULL;

void uiMain(void)
{
	iteration = gtk_main_iteration_do;
	gtk_main();
}

static gboolean stepsQuit = FALSE;

// the only difference is we ignore the return value from gtk_main_iteration_do(), since it will always be TRUE if gtk_main() was never called
// gtk_main_iteration_do() will still run the main loop regardless
static gboolean stepsIteration(gboolean block)
{
	gtk_main_iteration_do(block);
	return stepsQuit;
}

void uiMainSteps(void)
{
	iteration = stepsIteration;
}

int uiMainStep(int wait)
{
	gboolean block;

	block = FALSE;
	if (wait)
		block = TRUE;
	return (*iteration)(block) == FALSE;
}

// gtk_main_quit() may run immediately, or it may wait for other pending events; "it depends" (thanks mclasen in irc.gimp.net/#gtk+)
// PostQuitMessage() on Windows always waits, so we must do so too
// we'll do it by using an idle callback
static gboolean quit(gpointer data)
{
	if (iteration == stepsIteration)
		stepsQuit = TRUE;
		// TODO run a gtk_main() here just to do the cleanup steps of syncing the clipboard and other stuff gtk_main() does before it returns
	else
		gtk_main_quit();
	return FALSE;
}

void uiQuit(void)
{
	gdk_threads_add_idle(quit, NULL);
}

struct queued {
	void (*f)(void *);
	void *data;
};

static gboolean doqueued(gpointer data)
{
	struct queued *q = (struct queued *) data;

	(*(q->f))(q->data);
	g_free(q);
	return FALSE;
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	struct queued *q;

	// we have to use g_new0()/g_free() because uiprivAlloc() is only safe to call on the main thread
	// for some reason it didn't affect me, but it did affect krakjoe
	q = g_new0(struct queued, 1);
	q->f = f;
	q->data = data;
	gdk_threads_add_idle(doqueued, q);
}


int uiEventsPending() {
	return gtk_events_pending();
}


int waitForNodeEvents(uv_loop_t *loop, int timeout) {
	int nodeBackendFd = uv_backend_fd(loop);

	if (nodeBackendFd == -1) {
		fprintf(stderr, "Invalid node backend fd.\n");
		return 0;
	}

	struct epoll_event ev;

	return epoll_wait(nodeBackendFd, &ev, 1, timeout);
}


int uiLoopWakeup() {
	g_main_context_wakeup(NULL);
	return 0;
}