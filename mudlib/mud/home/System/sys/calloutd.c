/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kernel/rsrc.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

int suspend;			/* callouts suspended */
int handle;			/* releaser handle */
int dead;			/* if we're being destroyed */

object cmap;	/* ([ oindex : ([ handle : iterator ]) ]) */
object cqueue;	/* ({ iterator : ({ obj, handle }) }) */

int begin, end;

int empty();

private mixed *release();
private void alloc_queue();
private void free_queue();

/* private */

static void create()
{
	RSRCD->set_suspension_manager(this_object());
}

static void destruct()
{
	dead = 1;

	RSRCD->release_callouts();
}

private int object_index(object obj)
{
	int oindex;

	if (sscanf(object_name(obj), "%*s#%d", oindex)) {
		return oindex;
	}

	return status(obj, O_INDEX);
}

private void alloc_queue()
{
	cmap = clone_object(BIGSTRUCT_ARRAY_OBJ);
	cmap->claim();
	cqueue = clone_object(BIGSTRUCT_ARRAY_OBJ);
	cqueue->claim();

	cmap->set_size(status(ST_OTABSIZE));
	cqueue->set_size(0x40000000);
}

private void free_queue()
{
	destruct_object(cmap);
	destruct_object(cqueue);
}

/* initd hooks */

void reboot()
{
	if (cmap) {
		cmap->set_size(status(ST_OTABSIZE));
	}
}

/* rsrcd hooks */

void suspend_callouts()
{
	if (previous_program() == RSRCD) {
		if (dead) {
			return;
		}

		LOGD->post_message("system", LOG_NOTICE, "Suspending callouts");

		suspend = -1;

		if (handle) {
			remove_call_out(handle);
			handle = 0;
		}

		if (!cmap) {
			alloc_queue();
		}
	} else {
		RSRCD->suspend_callouts();
	}
}

void suspend(object obj, int handle)
{
	mixed *callout;

	mapping map;
	int oindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (dead) {
		return;
	}

	if (object_name(obj) == SUSPENDD) {
		catch {
			RSRCD->release_callout(obj, handle);
		}
		return;
	}

	if ((end + 1) & 0x3FFFFFFF == begin) {
		error("Suspension queue overflow");
	}

	oindex = object_index(obj);

	map = cmap->query_element(oindex);

	if (!map) {
		map = ([ ]);
		cmap->set_element(oindex, map);
	}

	map[handle] = end;

	cqueue->set_element(end++, ({ obj, handle }) );
	end &= 0x3FFFFFFF;
}

int remove_callout(object obj, int handle)
{
	mapping map;
	int oindex;
	int qindex;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (dead) {
		return TRUE;
	}

	oindex = object_index(obj);

	map = cmap->query_element(oindex);

	if (!map || map[handle] == nil) {
		return FALSE;
	}

	qindex = map[handle];
	map[handle] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	cqueue->set_element(qindex, nil);

	return TRUE;
}

void remove_callouts(object obj)
{
	mapping map;
	int oindex;
	int *handles;
	int *qindices;
	int sz;
	int osz;

	ACCESS_CHECK(previous_program() == RSRCD);

	if (dead) {
		return;
	}

	oindex = object_index(obj);

	map = cmap->query_element(oindex);

	if (!map) {
		return;
	}

	cmap->set_element(oindex, nil);

	qindices = map_values(map);

	osz = sizeof(qindices);

	for (sz = osz; --sz >= 0; ) {
		cqueue->set_element(qindices[sz], nil);
	}
}

void release_callouts()
{
	if (previous_program() == RSRCD) {
		if (dead) {
			return;
		}

		LOGD->post_message("system", LOG_NOTICE, "Releasing callouts");

		suspend = 1;

		/* Don't send any more callouts our way */
		RSRCD->release_callout(nil, 0);

		/* start draining backed up callouts */
		if (!handle) {
			handle = call_out("do_release", 0);
		}
	} else {
		RSRCD->release_callouts();
	}
}

/* public */

int query_suspend()
{
	return suspend;
}

/* internal */

static void do_release()
{
	mixed *callout;
	object obj;
	int ohandle;

	handle = 0;

	switch(suspend) {
	case -1: /* suspended */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while suspended");
		return;

	case 0: /* idle */
		LOGD->post_message("system", LOG_INFO, "calloutd: Release while idle");
		return;

	case 1: /* draining */
		callout = release();

		if (callout) {
			obj = callout[0];
			ohandle = callout[1];

			handle = call_out("do_release", 0);

			if (obj) {
				RSRCD->release_callout(obj, ohandle);
			}
		} else {
			RSRCD->release_callout(nil, 0);
			LOGD->post_message("system", LOG_NOTICE, "Released callouts");

			if (cmap) {
				free_queue();
			}

			suspend = 0;
			begin = end = 0;
		}

		break;
	}
}

private mixed *release()
{
	mixed *callout;
	mixed map;
	int oindex;

	if (begin == end) {
		return nil;
	}

	callout = cqueue->query_element(begin);
	cqueue->set_element(begin++, nil);

	begin &= 0x3FFFFFFF;

	if (!callout) {
		return ({ nil, -1 });
	}

	if (!callout[0]) {
		return ({ nil, -1 });
	}

	oindex = object_index(callout[0]);
	map = cmap->query_element(oindex);

	ASSERT(map);
	ASSERT(map[callout[1]] != nil);

	map[callout[1]] = nil;

	if (!map_sizeof(map)) {
		cmap->set_element(oindex, nil);
	}

	return callout;
}
