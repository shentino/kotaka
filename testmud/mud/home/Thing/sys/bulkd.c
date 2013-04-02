/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>
#include <thing/paths.h>
#include <status.h>

int handle;
object queue;

static void create()
{
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
}

void bulk_queue(object obj)
{
	ACCESS_CHECK(previous_program() == LIB_BULK || INTERFACE());

	queue->push_back(obj);

	if (!handle) {
		handle = call_out("process", 0);
	}
}

static void process()
{
	object obj;

	handle = 0;

	obj = queue->get_front();
	queue->pop_front();

	if (!queue->empty()) {
		LOGD->post_message("bulk", LOG_DEBUG, queue->get_size() + " objects left to sync.");
		handle = call_out("process", 0);
	}

	obj->bulk_sync();
}

void reset()
{
	mixed *co;
	int sz, i;

	ACCESS_CHECK(INTERFACE());

	handle = 0;

	queue = new_object(BIGSTRUCT_DEQUE_LWO);

	co = status(this_object(), O_CALLOUTS);
	sz = sizeof(co);

	for (i = 0; i < sz; i++) {
		if (co[i][CO_FUNCTION] == "process") {
			remove_call_out(co[i][CO_HANDLE]);
		}
	}
}
