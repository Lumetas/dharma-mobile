#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "events.h"

#define MAX_CALLBACKS 64
#define MAX_SCRIPT_PATH 512

static EventHandler *handlers = NULL;

void
events_init(void)
{
    handlers = NULL;
}

void
events_cleanup(void)
{
    EventHandler *h = handlers;
    while (h) {
        EventHandler *next = h->next;
        EventCallback *cb = h->callbacks;
        while (cb) {
            EventCallback *next_cb = cb->next;
            free(cb->script);
            free(cb);
            cb = next_cb;
        }
        free(h->name);
        free(h);
        h = next;
    }
    handlers = NULL;
}

static EventHandler*
find_handler(const char *name)
{
    EventHandler *h = handlers;
    while (h) {
        if (strcmp(h->name, name) == 0) {
            return h;
        }
        h = h->next;
    }
    return NULL;
}

static EventHandler*
add_handler(const char *name)
{
    EventHandler *h = find_handler(name);
    if (h) {
        return h;
    }
    
    h = malloc(sizeof(EventHandler));
    if (!h) return NULL;
    
    h->name = strdup(name);
    h->callbacks = NULL;
    h->next = handlers;
    handlers = h;
    return h;
}

int
events_register(const char *event_name, const char *script_path)
{
    EventHandler *h = add_handler(event_name);
    if (!h) return -1;
    
    EventCallback *cb = h->callbacks;
    int count = 0;
    while (cb) {
        if (strcmp(cb->script, script_path) == 0) {
            return 0;
        }
        cb = cb->next;
        count++;
    }
    
    if (count >= MAX_CALLBACKS) {
        return -1;
    }
    
    cb = malloc(sizeof(EventCallback));
    if (!cb) return -1;
    
    cb->script = strdup(script_path);
    cb->next = h->callbacks;
    h->callbacks = cb;
    return 0;
}

int
events_unregister(const char *event_name, const char *script_path)
{
    EventHandler *h = find_handler(event_name);
    if (!h) return 0;
    
    EventCallback **prev = &h->callbacks;
    EventCallback *cb = h->callbacks;
    
    while (cb) {
        if (strcmp(cb->script, script_path) == 0) {
            *prev = cb->next;
            free(cb->script);
            free(cb);
            return 0;
        }
        prev = &cb->next;
        cb = cb->next;
    }
    return 0;
}

void
events_trigger(const char *event_name, const char *arg1, const char *arg2, const char *arg3)
{
    EventHandler *h = find_handler(event_name);
    if (!h) return;
    
    EventCallback *cb = h->callbacks;
    while (cb) {
        char cmd[MAX_SCRIPT_PATH + 256];
        if (arg3) {
            snprintf(cmd, sizeof(cmd), "%s %s %s %s %s &", cb->script, event_name, arg1 ? arg1 : "", arg2 ? arg2 : "", arg3 ? arg3 : "");
        } else if (arg2) {
            snprintf(cmd, sizeof(cmd), "%s %s %s %s &", cb->script, event_name, arg1 ? arg1 : "", arg2 ? arg2 : "");
        } else if (arg1) {
            snprintf(cmd, sizeof(cmd), "%s %s %s &", cb->script, event_name, arg1 ? arg1 : "");
        } else {
            snprintf(cmd, sizeof(cmd), "%s %s &", cb->script, event_name);
        }
        system(cmd);
        cb = cb->next;
    }
}

void
events_list(char *buf, size_t bufsize)
{
    buf[0] = '\0';
    EventHandler *h = handlers;
    while (h) {
        EventCallback *cb = h->callbacks;
        while (cb) {
            char entry[MAX_SCRIPT_PATH + 64];
            snprintf(entry, sizeof(entry), "%s|%s\n", h->name, cb->script);
            strncat(buf, entry, bufsize - strlen(buf) - 1);
            cb = cb->next;
        }
        h = h->next;
    }
}
