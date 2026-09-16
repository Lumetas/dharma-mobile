#ifndef DHARMA_EVENT_H
#define DHARMA_EVENT_H

typedef enum {
	EVT_VIEW,
	EVT_TAG,
	EVT_TOGGLETAG,
	EVT_TOGGLEVIEW,
	EVT_FOCUS,
	EVT_UNFOCUS,
	EVT_MANAGE,
	EVT_UNMANAGE,
	EVT_SWITCHMON,
	EVT_TOGGLEBAR,
	EVT_TOGGLEFLOAT,
	EVT_FULLSCREEN,
	EVT_KILLCLIENT,
	EVT_LAST
} EventType;

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

void event_register(EventType type, void (*callback)(const Arg *, void *), void *client_data);
void event_unregister(EventType type, void (*callback)(const Arg *, void *));
void event_emit(EventType type, const Arg *arg);

#endif /* DHARMA_EVENT_H */
