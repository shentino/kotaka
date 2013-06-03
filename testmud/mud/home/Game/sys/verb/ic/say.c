/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_ENGVERB;

mixed **query_roles()
{
	return ({
		({ "iob", ({ "to" }), 0 })
	});
}

private object *filter_mobiles(object *bodies)
{
	object *mobiles;
	int sz, i;

	sz = sizeof(bodies);
	mobiles = ({ });

	for (i = 0; i < sz; i++) {
		mobiles |= bodies[i]->query_property("mobiles");
	}

	return mobiles;
}

void emit_say(object actor, object target, object listener, string evoke)
{
	string message;
	object body;

	body = listener->query_body();

	listener->message(evoke);
}

void do_action(object actor, mapping roles, string evoke)
{
	object user;
	object target;
	string name;

	int sz, i;

	object env;
	object *listeners;
	object *mobiles;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	user = query_user();

	if (!evoke) {
		send_out("Cat got your tongue?\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("You open your mouth to speak\nbut the emptiness around you muffles your words.\n");
		return;
	}

	listeners = env->query_inventory();
	mobiles = filter_mobiles(listeners);

	if (roles["iob"]) {
		target = roles["iob"][1];
	}

	sz = sizeof(mobiles);

	for (i = 0; i < sz; i++) {
		emit_say(actor, target, mobiles[i], evoke);
	}
}
