/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	int sz;
	string *resources;
	object proxy;
	string args;

	if (query_user()->query_class() < 3) {
		send_out("Permission denied.\n");
		return;
	}

	args = roles["raw"];

	resources = KERNELD->query_resources();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	for (sz = sizeof(resources), --sz; sz >= 0; --sz) {
		proxy->rsrc_set_limit(args, resources[sz], -1);
	}

	send_out("Resource limits for " + args + " removed.\n");
}
