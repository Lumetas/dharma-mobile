#ifndef DHARMA_EVENTS_H
#define DHARMA_EVENTS_H

#include <X11/Xlib.h>

typedef struct EventHandler EventHandler;
typedef struct EventCallback EventCallback;

struct EventCallback {
    char *script;
    EventCallback *next;
};

struct EventHandler {
    char *name;
    EventCallback *callbacks;
    EventHandler *next;
};

extern int control_fd;

void events_init(void);
void events_cleanup(void);
int events_handle_command(const char *cmd);
void events_trigger(const char *event_name, const char *arg1, const char *arg2, const char *arg3);
void events_list(char *buf, size_t bufsize);
int events_accept_connection(void);
void events_process_command(int fd);
int events_register(const char *event_name, const char *script_path);
int events_unregister(const char *event_name, const char *script_path);

#endif
