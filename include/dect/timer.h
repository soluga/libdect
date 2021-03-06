/*
 * libdect timer handling
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_TIMER_H
#define _LIBDECT_DECT_TIMER_H

extern struct dect_timer *dect_timer_alloc(const struct dect_handle *dh);
extern void dect_timer_free(const struct dect_handle *dh, struct dect_timer *timer);
extern void dect_timer_setup(struct dect_timer *timer,
			     void (*cb)(struct dect_handle *, struct dect_timer *),
			     void *data);
extern void dect_timer_start(const struct dect_handle *dh,
			     struct dect_timer *timer, unsigned int timeout);
extern void dect_timer_stop(const struct dect_handle *dh, struct dect_timer *timer);
extern bool dect_timer_running(const struct dect_timer *timer);

extern void *dect_timer_data(const struct dect_timer *timer);

#endif /* _LIBDECT_DECT_TIMER_H */
