/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <status.h>
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit "~/lib/utility/secretlog";

mapping facilities;

/*
targets:

prefix:info

([ facilities: ([ target: mask ]) ])

Prefixes:

null: ignore the message
	Used as a placeholder for "explicit nop" that will suppress
	the default "shout to driver" done when no targets are present.
file: write to a file
channel: post to a channel
kuser: message a klib user

*/

static void create()
{
	facilities = ([ ]);

	INITD->configure_logging();
}

void clear_targets()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	facilities = ([ ]);
}

void set_target(string facility, int mask, string target_info)
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	CHECKARG(facility, 1, "set_target");
	CHECKARG(facility != "", 1, "set_target");
	CHECKARG(mask >= 0, 2, "set_target");
	CHECKARG(mask <= 255, 2, "set_target");
	CHECKARG(target_info, 3, "set_target");
	CHECKARG(target_info != "", 3, "set_target");

	if (mask) {
		mapping targets;

		targets = facilities[facility];

		if (!targets) {
			facilities[facility] = targets = ([ ]);
		}

		targets[target_info] = mask;
	} else {
		mapping targets;

		targets = facilities[facility];

		if (!targets) {
			return;
		}

		targets[target_info] = nil;

		if (!map_sizeof(targets)) {
			facilities[facility] = nil;
		}
	}
}

/*
000000000011111111112222
012345678901234567890123
WWW MMM DD HH:MM:SS YYYY

MMM DD HH:MM
MMM DD  YYYY
*/

private void send_to_target(string target, string timestamp, string message)
{
	string prefix, info;
	int sz, usz, i, j;
	mixed users;

	sscanf(target, "%s:%s", prefix, info);

	if (!prefix) {
		ASSERT(!info);
		prefix = target;
	}

	while (message) {
		string line;

		if (!sscanf(message, "%s\n%s", line, message)) {
			line = message;
			message = nil;
		}

		users = ([ ]);

		switch(prefix) {
		case "null":
			break;

		case "driver":
			DRIVER->message(line + "\n");
			break;

		case "channel":
			ASSERT(info);

			if (find_object(CHANNELD)) {
				CHANNELD->post_message(info, nil, line);
			}

			break;

		case "file":
			ASSERT(info);

			write_secret_log(info, timestamp + " " + line);

			break;

		case "user":
			if (this_user()) {
				users[this_user()] = 1;
			}
			break;

		case "kusers":
			{
				object *kusers;
				int i;
				int ksz;

				kusers = users();

				ksz = sizeof(users);

				for (i = 0; i < ksz; i++) {
					users[kusers[i]] = 1;
				}
			}
			break;

		case "kwizards":
			{
				string *kwizards;
				object *kusers;
				int i;
				int ksz;

				kwizards = KERNELD->query_users();
				kusers = users();

				ksz = sizeof(kusers);

				for (i = 0; i < ksz; i++) {
					if (
						sizeof(
							({ kusers[i]->query_name() })
							& kwizards
						)
					) {
						users[kusers[i]] = 1;
					}
				}
			}
			break;

		case "kadmins":
			{
				string *kwizards;
				object *kusers;
				int i;
				int ksz;

				kusers = users();
				ksz = sizeof(kusers);

				for (i = 0; i < ksz; i++) {
					string username;

					username = kusers[i]->query_name();

					if (KERNELD->access(username, "/", FULL_ACCESS)) {
						users[kusers[i]] = 1;
					}
				}
			}
			break;

		case "kadmin":
			{
				object kadmin;

				kadmin = KERNELD->find_user("admin");

				if (kadmin) {
					users[kadmin] = 1;
				}
			}
			break;

		default:
			DRIVER->message("Unparseable log target: " + target + "\n");
		}

		users = map_indices(users);
		usz = sizeof(users);

		for (i = 0; i < usz; i++) {
			object user;

			user = users[i];

			catch {
				user->message(line + "\n");
			}
		}
	}
}

void post_message(string facility, int priority, string message)
{
	string creator;
	mapping hits;
	string timestamp;

	CHECKARG(facility, 1, "post_message");
	CHECKARG(facility != "", 1, "post_message");
	CHECKARG(priority >= 0, 2, "post_message");
	CHECKARG(priority <= 7, 2, "post_message");
	CHECKARG(message, 3, "post_message");

	catch {
		timestamp = timestamp();
		hits = ([ ]);

		if (facilities[facility]) {
			mapping submap;
			string *targets;
			int *masks;
			int index;
			int sz;

			submap = facilities[facility];
			targets = map_indices(submap);
			masks = map_values(submap);

			sz = map_sizeof(submap);

			for (index = 0; index < sz; index++) {
				if (masks[index] & (1 << priority)) {
					hits[targets[index]] = 1;
				}
			}
		}

		if (!hits["null"] && facilities["*"]) {
			mapping submap;
			string *targets;
			int *masks;
			int index;
			int sz;

			submap = facilities["*"];
			targets = map_indices(submap);
			masks = map_values(submap);

			sz = map_sizeof(submap);

			for (index = 0; index < sz; index++) {
				if (masks[index] & (1 << priority)) {
					hits[targets[index]] = 1;
				}
			}
		}

		creator = DRIVER->creator(object_name(previous_object()));

		if (map_sizeof(hits)) {
			string *targets;
			int sz;
			int index;

			targets = map_indices(hits);
			sz = sizeof(targets);

			for (index = 0; index < sz; index++) {
				catch {
					send_to_target(targets[index], timestamp, message);
				}
			}
		} else {
			if (strlen(message) > 200) {
				string head, tail;

				head = message[.. 99];
				tail = message[strlen(message) - 100 ..];

				message = head + " ... " + tail;
			}

			DRIVER->message(facility + " " + priority + ": " + message + "\n");
		}
	} : {
		if (strlen(message) > 200) {
			string head, tail;

			head = message[.. 99];
			tail = message[strlen(message) - 100 ..];

			message = head + " ... " + tail;
		}

		DRIVER->message("Error logging: " + creator + ": " + facility + ": " + message + "\n");
	}
}
