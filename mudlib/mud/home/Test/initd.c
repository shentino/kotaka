/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("sys", 1);
}

private void set_limits()
{
	KERNELD->rsrc_set_limit("Test", "ticks", 100000000);
}

static void create()
{
	MODULED->boot_module("Bigstruct");

	KERNELD->set_global_access("Test", 1);

	set_limits();

	load();

	call_out("test", 0);
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	set_limits();

	load();

	purge_orphans("Test");

	call_out("test", 0);
}

static void test()
{
	"sys/bigstruct"->test();
	"sys/suspend"->test();
}
