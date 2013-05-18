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

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	object user;
	string name;

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to speak.\n");
		return;
	}

	args = STRINGD->trim_whitespace(args);

	if (!args || args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());

	send_out("You say: " + args + "\n");
	TEXT_SUBD->send_to_all_except(name + " says: " + args + "\n", ({ user }) );
}
