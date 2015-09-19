/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

void load()
{
	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("sys", 1);
}

static void create()
{
	KERNELD->set_global_access("Text", 1);

	MODULED->boot_module("Ansi");
	MODULED->boot_module("Verb");

	load();
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->save();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->restore();
}

void hotboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	"sys/aliasd"->restore();
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();

	purge_orphans("Text");
}
