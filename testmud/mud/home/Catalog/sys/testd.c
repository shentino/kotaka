/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/assert.h>

static void create()
{
}

void test()
{
	"catalogd"->add_object("Test:foo:baR", this_object());
	"catalogd"->add_object("Test:foo:bar", this_object());
	"catalogd"->add_object("Test:foo:bR", this_object());
	"catalogd"->add_object("Test:foo:R", this_object());
	"catalogd"->dump();

	"catalogd"->remove_object("Test:foo:R");
	"catalogd"->dump();

	"catalogd"->remove_object("Test:foo:bR");
	"catalogd"->dump();

	"catalogd"->remove_object("Test:foo:bar");
	"catalogd"->dump();

	ASSERT("catalogd"->lookup_object("Test:foo:baR") == this_object());
	"catalogd"->remove_object("Test:foo:baR");
	"catalogd"->dump();
	ASSERT("catalogd"->lookup_object("Test:foo:baR") == nil);
}
