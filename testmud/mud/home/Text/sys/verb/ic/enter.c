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
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object env;
	object *shacks;

	int ax, ay;

	int i, sz;

	env = actor->query_environment();

	if (!env) {
		send_out("You enter the world.\n");
		actor->move(GAME_INITD->query_world());
		return;
	}

	shacks = env->query_inventory();

	/* first, check for shacks */
	sz = sizeof(shacks);

	for (i = 0; i < sz; i++) {
		if (shacks[i]->query_property("id") != "shack") {
			shacks[i] = nil;
		}
	}

	shacks -= ({ nil });

	if (!sizeof(shacks)) {
		send_out("There are no shacks present for you to enter.\n");
		return;
	}

	/* next, check to see if any are in range */
	sz = sizeof(shacks);

	ax = actor->query_x_position();
	ay = actor->query_y_position();

	for (i = 0; i < sz; i++) {
		int dx, dy;

		dx = shacks[i]->query_x_position() - ax;
		dy = shacks[i]->query_y_position() - ay;

		if (dx < -3 || dx > 0 || dy < 0 || dy > 3) {
			shacks[i] = nil;
		}
	}

	shacks -= ({ nil });

	switch(sizeof(shacks)) {
	case 0:
		send_out("The only shacks around are not in range.\n");
		break;
	case 1:
		send_out("You enter the shack.\n");
		actor->move(shacks[0]);
		break;
	default:
		send_out("There is more than one shack in range.  Randomly entering one.\n");

		shacks[0] = shacks[random(sizeof(shacks))];

		break;
	}
}
