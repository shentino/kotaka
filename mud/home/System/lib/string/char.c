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
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;

string chars(int code, int count)
{
	string log;

	if (code < 0 || code > 0xff) {
		error("Invalid character code");
	}

	log = " ";
	log[0] = code;

	while (strlen(log) < count) {
		if (strlen(log) * 2 > count) {
			log += log[0 .. count - strlen(log) - 1];
		} else {
			log += log;
		}
	}

	return log[0 .. count - 1];
}

string spaces(int count)
{
	return chars(' ', count);
}

string nulls(int count)
{
	return chars('\0', count);
}
