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
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

int query_raw()
{
	return 1;
}

void main(object actor, mixed *tree)
{
	object turkey;
	object user;
	string kicker_name;
	string turkey_name;
	string args;

	args = fetch_raw(tree);

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can unban someone.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to unban?\n");
		return;
	}

	if (!BAND->query_is_username_banned(args)) {
		send_out("That user is not banned.\n");
		return;
	}

	switch("~/sys/subd"->query_user_class(args)) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can unban an administrator.");
			return;
		}
		break;
	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can unban a wizard.");
			return;
		}
		break;
	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can unban someone.");
			return;
		}
		break;
	}

	BAND->unban_username(args);

	kicker_name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());
	turkey_name = TEXT_SUBD->titled_name(args, "~/sys/subd"->query_user_class(args));

	user->message("You unban " + turkey_name + " from the mud.\n");

	TEXT_SUBD->send_to_all_except(kicker_name + " unbans " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );
}
