/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

string query_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "Welcome to Kotaka\n";
}

string query_blocked_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is undergoing maintenance.\n";
}

string query_overload_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is too full to accept any more connections.\n";
}

string query_sitebanned_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "Banned.\n";
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return 60;
}

object select(string str)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return find_object(SYSTEM_USERD);
}

void connect(string host, int port)
{
	object conn;

	conn = clone_object(SYSTEM_CONN);
	conn->set_manager(this_object());
	conn->connect(host, port);
}

static int is_sitebanned(string ip)
{
	string o1, o2, o3, o4;

	if (!find_object(BAND)) {
		return 0;
	}

	if (sscanf(ip, "%s.%s.%s.%s", o1, o2, o3, o4) < 4) {
		/* weird IP? */
		return 0;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + "." + o3 + "." + o4)) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + "." + o3 + ".*")) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + ".*.*")) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + ".*.*.*")) {
		return 1;
	}

	return 0;
}
