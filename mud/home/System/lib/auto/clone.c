/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

private object prev, next;

nomask void set_prev_clone(object new_prev)
{
	ACCESS_CHECK(SYSTEM());

	prev = new_prev;
}

nomask void set_next_clone(object new_next)
{
	ACCESS_CHECK(SYSTEM());

	next = new_next;
}

nomask object query_prev_clone()
{
	return prev;
}

nomask object query_next_clone()
{
	return next;
}
