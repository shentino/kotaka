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
#include <kotaka/paths/account.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

#define STATE_GETNAME	1
#define STATE_GETPASS	2
#define STATE_CHKPASS	3

string name;
string password;

int state;
int stopped;
int reading;
int dead;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	switch(state) {
	case STATE_GETNAME:
		send_out("Please choose a username: ");
		break;

	case STATE_GETPASS:
		send_out("Please choose a password: ");
		break;

	case STATE_CHKPASS:
		send_out("Please confirm your password: ");
		break;
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	if (query_user()->query_username()) {
		send_out("You are already logged in.\n");
		pop_state();
		return;
	}

	state = STATE_GETNAME;
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

void pre_end()
{
	ACCESS_CHECK(previous_object() == query_user());

	dead = 1;
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(state) {
	case STATE_GETNAME:
		input = STRINGD->to_lower(input);
		if (!STRINGD->is_valid_username(input)) {
			send_out("That is not a valid username.\n");
			pop_state();
			return;
		} else if (ACCOUNTD->query_is_registered(input)) {
			send_out("That name is already taken.\n");
			pop_state();
			return;
		} else {
			state = STATE_GETPASS;
			query_user()->set_mode(MODE_NOECHO);
			name = input;
			break;
		}
		break;

	case STATE_GETPASS:
		send_out("\n");
		if (ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, someone else just swiped the username you wanted.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (BAND->query_is_user_banned(name)) {
			send_out("That name is banned.\n");
			query_user()->quit();
			return;
		} else {
			password = input;
			state = STATE_CHKPASS;
			break;
		}
		break;

	case STATE_CHKPASS:
		send_out("\n");
		if (ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, someone else just swiped the username you wanted.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (BAND->query_is_user_banned(name)) {
			send_out("Whoops, the username you picked just got banned.\n");
			query_user()->quit();
			return;
		} else if (input != password) {
			send_out("Password mismatch.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else {
			object parent;
			object pager;
			string text;

			ACCOUNTD->register_account(name, password);

			query_user()->set_username(name);

			ACCOUNTD->set_account_property(name, "channels", ({ "chat" }));

			TEXT_SUBD->login_user(user);

			TEXT_SUBD->send_register_message(name);

			pager = clone_object("page");
			text = read_file("~/data/quickstartguide");

			pager->set_text(
			text ? text : "Yell at the admin, he broke the quick start guide."
			);

			parent = query_parent();

			if (instanceof(parent, "start")) {
				object shell;

				suspend_user();
				shell = clone_object("shell");
				push_state(shell);
				shell->push_state(pager);
				release_user();
				parent->collapse_state(this_object());
				collapse_state(shell);
			} else {
				swap_state(pager);
			}

			return;
		}
		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

void end()
{
	destruct_object(this_object());
}