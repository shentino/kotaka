/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2001, 2007, 2008, 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/tls.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/string.h>

inherit SECOND_AUTO;

private inherit API_TLS;

static void create()
{
	::create();
	::set_tls_size(1);
}

mixed query_tls_value(string domain, string key)
{
	string creator;
	mapping heap;
	mapping dmap;

	creator = DRIVER->creator(object_name(previous_object()));

	if (domain != creator) {
		error("Access denied");
	}

	heap = get_tlvar(0);

	if (!heap) {
		return nil;
	}

	dmap = heap[domain];

	if (!dmap) {
		return nil;
	}

	return dmap[key];
}

void set_tls_value(string domain, string key, mixed value)
{
	string creator;
	mapping heap;
	mapping dmap;

	creator = DRIVER->creator(object_name(previous_object()));

	if (domain != creator) {
		error("Access denied");
	}

	heap = get_tlvar(0);

	if (!heap) {
		if (value == nil) {
			return;
		} else {
			heap = ([ ]);
		}
	}

	dmap = heap[domain];

	if (!dmap) {
		if (value == nil) {
			return;
		} else {
			dmap = ([ ]);
		}
	}

	dmap[key] = value;

	if (map_sizeof(dmap) == 0) {
		dmap = nil;
	}

	heap[domain] = dmap;

	if (map_sizeof(heap) == 0) {
		heap = nil;
	}

	set_tlvar(0, heap);
}