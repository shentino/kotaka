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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit "~/lib/user";

object mobile;
object body;
int keepalive;
int naws_callout;

string username;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

void dispatch_wiztool(string line)
{
	ACCESS_CHECK(TEXT());

	PROXYD->get_wiztool(query_name())->input(line);
}

static void destruct(int clone)
{
	if (clone) {
		::destruct();
	}
}

string query_username()
{
	return username;
}

int query_class()
{
	return TEXT_SUBD->query_user_class(username);
}

string query_titled_name()
{
	return TEXT_SUBD->query_titled_name(username);
}

void set_username(string new_username)
{
	ACCESS_CHECK(TEXT());

	username = new_username;
}

string query_name()
{
	return username;
}

void set_mobile(object new_mobile)
{
	ACCESS_CHECK(TEXT() || GAME() || VERB());

	mobile = new_mobile;
}

object query_mobile()
{
	return mobile;
}

void set_body(object new_body)
{
	object *mobiles;

	ACCESS_CHECK(TEXT() || GAME() || VERB());

	if (body) {
		mobiles = body->query_property("mobiles");

		if (mobiles) {
			mobiles -= ({ this_object(), nil });

			if (!sizeof(mobiles)) {
				mobiles = nil;
			}

			body->set_property("mobiles", mobiles);
		}
	}

	body = new_body;

	if (body) {
		mobiles = body->query_property("mobiles");

		if (!mobiles) {
			mobiles = ({ });
		}

		mobiles -= ({ nil });
		mobiles |= ({ this_object() });
		body->set_property("mobiles", mobiles);
	}
}

object query_body()
{
	return body;
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	::logout(quit);
}

private void do_login()
{
	connection(previous_object());

	set_mode(MODE_ECHO);

	set_root_state(new_object("~/lwo/ustate/start"));

	TEXT_USERD->add_guest(this_object());
}

private int do_receive(string str)
{
	int ret;
	object conn, conn2;
	string ip, user;

	conn = previous_object();

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	ip = query_ip_number(conn);

	if (!ip)
		ip = "nil";

	user = username ? username : ip;

	SECRETD->make_dir(".");
	SECRETD->make_dir("log");
	SECRETD->write_file("log/log-" + user, "[" + SUBD->pmtime(millitime()) + "] " + user + ": " + str + "\n");

	ret = ::receive_message(str);

	if (ret == MODE_DISCONNECT) {
		INITD->message("User is dying...");
	}

	set_mode(ret);

	return ret;
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	do_login();

	if (str != nil) {
		return do_receive(str);
	}

	return MODE_NOCHANGE;
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return do_receive(str);
}

void quit()
{
	disconnect();
}

void channel_message(string channel, string stamp, string sender, string message)
{
	ACCESS_CHECK(previous_program() == CHANNELD);

	if (sender) {
		if (message) {
			send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
				"\033[0m \033[1;34m" + sender + "\033[0m: " + message + "\n");
		} else {
			send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
				"\033[0m \033[1;34m" + sender + "\033[0m\n");
		}
	} else {
		send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
			"\033[0m " + message + "\n");
	}
}

void subscribe_channels()
{
	string *channels;

	channels = ACCOUNTD->query_account_property(username, "channels");

	if (channels) {
		int sz;

		for (sz = sizeof(channels); --sz >= 0; ) {
			if (CHANNELD->test_channel(channels[sz])) {
				CHANNELD->subscribe_channel(channels[sz], this_object());
			} else {
				message("Warning: " + channels[sz] + " does not exist.\n");
			}
		}
	}
}

static object telnet_obj()
{
	object conn;

	conn = query_conn();

	while (conn && conn <- LIB_USER) {
		if (conn <- "~Text/obj/filter/telnet") {
			return conn;
		}
		conn = conn->query_conn();
	}
}

static void process_naws()
{
	call_out("process_naws", 0.1);

	telnet_obj()->send_do(31);
}

void do_naws(int enable)
{
	if (naws_callout) {
		remove_call_out(naws_callout);
		naws_callout = 0;
	}

	if (enable) {
		naws_callout = call_out("process_naws", 0);
	}
}
