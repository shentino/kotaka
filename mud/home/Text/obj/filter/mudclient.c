/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/telnet.h>

inherit LIB_FILTER;

int msp_pending;
int msp_active;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

int login(string str)
{
	previous_object()->message("Welcome to Shentino's mudclient extension.\n");

	return ::login(str);
}

void telnet_do(int code)
{
	switch(code) {
	case 90:
		if (msp_active) {
			/* ignore */
		} else if (msp_pending) {
			msp_active = 1;
			msp_pending = 0;
		} else {
			query_conn()->send_will(90);
			msp_active = 1;
		}
		break;

	default:
		query_conn()->send_wont(code);
	}
}

void telnet_dont(int code)
{
}

void telnet_will(int code)
{
	switch(code) {
	default:
		query_conn()->send_dont(code);
	}
}

void telnet_wont(int code)
{
}

void telnet_subnegotiation(int code)
{
}

void enable_msp()
{
	msp_pending = 1;

	query_conn()->send_will(90);
}

void disable_msp()
{
	query_conn()->send_dont(90);

	msp_pending = 0;
	msp_active = 0;
}

void beep()
{
	message("!!SOUND(beep.wav)\n");
}
