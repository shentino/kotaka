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
#include <kernel/user.h>
#include <kotaka/log.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/utility.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

/* Enable and redefine to suit */
/* Also inspect the query_banner function below to validate the information provided to the router */

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;
inherit "/lib/copy";
inherit "/lib/string/case";
inherit "/lib/string/sprint";
inherit "/lib/string/replace";

/* daemon state */
int handle;
int keepalive;
string buffer;
string mudname;
int rejections;

/* i3 interface */
int password;
int chanlistid;
int mudlistid;
string router;

/* local */
mapping routers;

/* i3 information */
mapping muds;
mapping channels;

static void save();
private void restore();

static void create()
{
	mudlistid = 0;
	chanlistid = 0;
	rejections = 0;
	router = "*wpr";

	routers = ([ ]);
	muds = ([ ]);
	channels = ([ ]);

	call_out("connect_i3", 0);

	restore();

	if (!router) {
		router = "*wpr";
		call_out("save", 0);
	}

	if (!routers || !map_sizeof(routers)) {
		call_out("reset_routers", 0);
	}
}

void listen_channel(string channel, int on);
private string mudmode_sprint(mixed data);

static void connect_i3()
{
	string ip;
	int port;

	if (!router) {
		error("No router selected");
	}

	if (!routers[router]) {
		error("No such router: " + router);
	}

	({ ip, port }) = routers[router];

	connect(ip, port);
}

private string make_packet(mixed *data)
{
	int len;
	string str;
	string bigendian;

	str = mudmode_sprint(data);
	len = strlen(str) + 1;

	bigendian = "    ";

	bigendian[0] = len >> 24;
	bigendian[1] = len >> 16;
	bigendian[2] = len >> 8;
	bigendian[3] = len;

	return bigendian + str + "\000";
}

private string mudmode_sprint(mixed data)
{
	int iter;
	string tmp;
	mixed *arr;

	switch (typeof(data)) {
	case T_NIL:
		return "0";

	case T_STRING:
		return "\"" + quote_escape(data) + "\"";

	case T_INT:
		return (string)data;

	case T_FLOAT:
		/* decimal point is required */
		{
			string mantissa;
			string exponent;
			string str;

			str = (string)data;

			if (!sscanf(str, "%se%s", mantissa, exponent)) {
				mantissa = str;
				exponent = "";
			} else {
				exponent = "e" + exponent;
			}

			if (!sscanf(mantissa, "%*s.")) {
				mantissa += ".0";
			}

			return mantissa + exponent;
		}

	case T_ARRAY:
		if (sizeof(data) == 0)
			return "({})";

		tmp = "({";
		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += mudmode_sprint(data[iter]) + ",";
		}
		return tmp + "})";

	case T_MAPPING:
		if (map_sizeof(data) == 0)
			return "([])";

		arr = map_indices(data);
		tmp = "([";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += mudmode_sprint(arr[iter]) + ":" +
				mudmode_sprint(data[arr[iter]]) + ",";
		}
		return tmp + "])";
	}
}

/* logging */

private void log_outbound(mixed *arr)
{
/*	LOGD->post_message("debug", LOG_DEBUG, "I3 packet out: " + mixed_sprint(arr));*/
}

private void log_inbound(mixed *arr)
{
/*	LOGD->post_message("debug", LOG_DEBUG, "I3 packet in: " + mixed_sprint(arr));*/
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!routers) {
		routers = ([ ]);
	}
}

private mixed *startup_packet()
{
	mudname = rejections ? "Kotaka" + (rejections + 1) : "Kotaka";

	return ({
		"startup-req-3",
		5,
		mudname,
		0,
		router,
		0,

		password,
		0,
		0,

		50000,
		0,
		0,
		"Kotaka",
		"Kotaka",
		status(ST_VERSION),
		"DGD",
		"(fill in the purpose of the mud here)",
		"(fill in your admin email here)",
		([
			"channel" : 1,
			"tell" : 1
		]),
		([ ])
	});

}

string query_banner(object LIB_CONN connection)
{
	mixed *arr;

	arr = startup_packet();

	log_outbound(arr);

	return make_packet(arr);
}

int query_timeout(object LIB_CONN connection)
{
	return 3;
}

object select(string input)
{
	return this_object();
}

void send_channel_message(string channel, string sender, string visible, string text)
{
	mixed *arr;

	arr = ({
		"channel-m",
		5,
		mudname,
		sender ? sender : 0,
		0,
		0,
		channel,
		visible,
		text
	});

	log_outbound(arr);

	message(make_packet(arr));
}

private void do_chanlist_reply(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	chanlistid = value[6];
	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);
	sz = sizeof(names);

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		if (values[sz] == 0) {
			channels[names[sz]] = nil;
		} else {
			channels[names[sz]] = values[sz];

			if (CHANNELD->query_intermud(names[sz])) {
				listen_channel(names[sz], 1);
			}
		}
	}

	call_out("save", 0);
}

private void do_emoteto(mixed *value)
{
	string mud;
	string message;
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		user->message(value[6] + "@" + value[2] + " emotes to you: " + value[7]);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			mudname,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		log_outbound(arr);

		message(make_packet(arr));
	}
}

private void do_error(mixed *value)
{
	if (value[2] == mudname) {
		if (value[6] == "keepalive") {
			return;
		}
	}

	LOGD->post_message("system", LOG_ERR, "IntermudD: I3 error: " + mixed_sprint(value));

	if (value[6] == "not-allowed") {
		if (value[7] == "Bad password") {
			mixed *badpkt;

			badpkt = value[8];

			if (badpkt[0] == "startup-req-3") {
				LOGD->post_message("system", LOG_ERR, "I3: Rejected for having a bad password, incrementing");

				rejections++;

				disconnect();

				buffer = "";

				call_out("connect_i3", 5);
			}
		}
	}
}

private void do_mudlist(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	mudlistid = value[6];
	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);
	sz = sizeof(names);

	for ( ; --sz >= 0; ) {
		if (values[sz] == 0) {
			muds[names[sz]] = nil;
		} else {
			muds[names[sz]] = values[sz];
		}
	}

	call_out("save", 0);
}

private void do_startup_reply(mixed *value)
{
	string *ch;
	mixed *raw;
	int sz;

	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Received startup reply");

	password = value[7];

	call_out("save", 0);

	ch = CHANNELD->query_channels();
	sz = sizeof(ch);

	for (sz = sizeof(ch); --sz >= 0; ) {
		if (channels[ch[sz]]) {
			listen_channel(ch[sz], CHANNELD->query_intermud(ch[sz]));
		}
	}

	keepalive = call_out("keepalive", 0);
}

private void do_tell(mixed *value)
{
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		string msg;

		msg = value[7];

		if (msg[strlen(msg) - 1] != '\n') {
			msg += "\n";
		}

		user->message(value[6] + "@" + value[2] + " tells you: " + msg);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			mudname,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		log_outbound(arr);

		message(make_packet(arr));
	}
}

private void do_channel_m(mixed *value)
{
	string name;
	string visname;
	string mud;
	string message;
	string newmessage;
	string header;
	string channel;
	int i, sz;
	int cflag;

	message = value[8];
	newmessage = "";

	for (sz = strlen(message), i = 0; i < sz; i++) {
		string tip;

		if (message[i] < ' ') {
			tip = "^@";
			tip[1] += message[i];
			cflag = 1;
		} else {
			tip = " ";
			tip[0] = message[i];
		}

		newmessage += tip;
	}

	message = newmessage;

	mud = value[2];
	name = value[3];
	channel = value[6];
	visname = value[7];

	if (sscanf(message, "%*s%%^")) {
		message = TEXT_SUBD->pinkfish2ansi(message) + "\033[0m";
	}

	if (cflag) {
		LOGD->post_message("system", LOG_WARNING, name + "@" + mud + " sent a control character on I3 via " + channel);
		LOGD->post_message("debug", LOG_DEBUG, name + "@" + mud + " sent " + message + " on " + channel);
	}

	if (CHANNELD->test_channel(channel)) {
		if (to_lower(name) == to_lower(visname)) {
			CHANNELD->post_message(channel, visname + "@" + mud, message, 1);
		} else {
			CHANNELD->post_message(channel, visname + " (" + name + ")@" + mud, message, 1);
		}
	}
}

private void do_channel_e(mixed *value)
{
	if (CHANNELD->test_channel(value[6])) {
		string text;

		text = replace(
			value[8], "$N", value[7] + "@" + value[2]);

		CHANNELD->post_message(value[6], text, nil, 1);
	}
}

private void do_who_reply(mixed *value)
{
	object user;
	string buffer;
	int i, sz;
	mixed *who_data;

	user = TEXT_USERD->find_user(value[5]);

	if (!user) {
		LOGD->post_message("system", LOG_NOTICE, "I3 sent a who-reply to offline user " + value[5]);
		return;
	}

	who_data = value[6];

	user->message("Users online at " + value[2] + ":\n");

	for (i = 0, sz = sizeof(who_data); i < sz; i++) {
		string visname;
		int idle;
		string extra;

		visname = who_data[i][0];
		idle = who_data[i][1];
		extra = who_data[i][2];

		user->message(TEXT_SUBD->pinkfish2ansi(visname));

		if (extra) {
			user->message(" - " + TEXT_SUBD->pinkfish2ansi(extra));
		}

		if (idle) {
			user->message(" (" + idle + "s)");
		}

		user->message("\n");
	}

	user->message("\033[0m");
}

private void bounce_packet(mixed *value)
{
	mixed *arr;

	/* send back an error packet */
	LOGD->post_message("system", LOG_ERR,
		"IntermudD: Unhandled packet:\n" + hybrid_sprint(value) + "\n");

	LOGD->post_message("system", LOG_ERR,
		"IntermudD: Bouncing back an error to \"" + value[2] + "\"\n");

	arr = ({
		"error",
		5,
		mudname,
		0,
		value[2],
		value[3],
		"unk-type",
		"Unhandled packet type: " + value[0],
		value
	});

	log_outbound(arr);

	message(make_packet(arr));
}

private void process_packet(mixed *value)
{
	switch(value[0]) {
	case "chanlist-reply":
		do_chanlist_reply(value);
		break;

	case "channel-m":
		do_channel_m(value);
		break;

	case "channel-e":
		do_channel_e(value);
		break;

	case "emoteto":
		do_emoteto(value);
		break;

	case "error":
		do_error(value);
		break;

	case "mudlist":
		do_mudlist(value);
		break;

	case "startup-reply":
		do_startup_reply(value);
		break;

	case "tell":
		do_tell(value);
		break;

	case "who-reply":
		do_who_reply(value);
		break;

	/* we don't care about these right now */
	case "finger-req": /* ignored for privacy */
	case "locate-req": /* ignored for privacy */
	case "ucache-update": /* we don't implement ucache */
	case "who-req": /* ignored for privacy */
		break;

	default:
		bounce_packet(value);
	}
}

static void process()
{
	mixed *arr;
	int len;
	string packet;

	handle = 0;

	if (strlen(buffer) < 4) {
		return;
	}

	len = buffer[3] + (buffer[2] << 8) + (buffer[1] << 16) + (buffer[0] << 24);

	if (strlen(buffer) < len + 4) {
		return;
	}

	buffer = buffer[4 ..];
	packet = buffer[0 .. len - 2];
	buffer = buffer[len ..];

	arr = PARSER_MUDMODE->parse(packet);

	log_inbound(arr);

	process_packet(arr);

	handle = call_out("process", 0);
}

static void keepalive()
{
	mixed *arr;

	keepalive = call_out("keepalive", 60);

	arr = ({
		"who-req",
		5,
		mudname,
		0,
		mudname,
		0
	});

	log_outbound(arr);

	message(make_packet(arr));
}

/* communication */

int login(string input)
{
	connection(previous_object());

	buffer = input;
	handle = call_out("process", 0);

	return MODE_NOCHANGE;
}

int receive_message(string input)
{
	buffer += input;

	if (!handle) {
		handle = call_out("process", 0);
	}

	return MODE_NOCHANGE;
}

int message_done()
{
	return MODE_NOCHANGE;
}

void logout(int quit)
{
	if (quit) {
		LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection closed");
	} else {
		LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection lost, reconnecting");

		call_out("connect_i3", 5);
	}

	if (handle) {
		remove_call_out(handle);
		handle = 0;
	}

	buffer = nil;
}

void connect_failed(int refused)
{
	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection failed");

	call_out("connect_i3", 30);
}

static void destruct()
{
	if (query_conn()) {
		disconnect();
	}
}

string *query_channels()
{
	return map_indices(channels);
}

mixed *query_channel(string channel)
{
	return deep_copy(channels[channel]);
}

string *query_muds()
{
	return map_indices(muds);
}

mixed *query_mud(string mud)
{
	return deep_copy(muds[mud]);
}

void send_who(string from, string mud)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"who-req",
		5,
		mudname,
		from,
		mud,
		0
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void send_tell(string from, string decofrom, string mud, string user, string message)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"tell",
		5,
		mudname,
		from,
		mud,
		user,
		decofrom,
		message
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void send_emote(string from, string decofrom, string mud, string user, string message)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"emoteto",
		5,
		mudname,
		from,
		mud,
		user,
		decofrom + "@" + mudname,
		message
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void listen_channel(string channel, int on)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || CHANNEL());

	arr = ({
		"channel-listen",
		5,
		mudname,
		0,
		router,
		0,
		channel,
		on
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void add_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	if (channels[channel] && channels[channel][0] != mudname) {
		error("Not our channel");
	}

	channels[channel] = ({ mudname, 0 });

	arr = ({
		"channel-add",
		5,
		mudname,
		0,
		router,
		0,
		channel,
		0
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void remove_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	if (!channels[channel]) {
		error("No such channel");
	}

	if (channels[channel][0] != mudname) {
		error("Not our channel");
	}

	channels[channel] = nil;

	arr = ({
		"channel-remove",
		5,
		mudname,
		0,
		router,
		0,
		channel
	});

	log_outbound(arr);

	message(make_packet(arr));
}

static void save()
{
	string buf;

	buf = hybrid_sprint( ([
		"chanlistid" : chanlistid,
		"mudlistid" : mudlistid,
		"password" : password,
		"router" : router,
		"routers" : routers
	]) );

	SECRETD->make_dir(".");
	SECRETD->remove_file("intermud-tmp");
	SECRETD->write_file("intermud-tmp", buf + "\n");
	SECRETD->remove_file("intermud");
	SECRETD->rename_file("intermud-tmp", "intermud");
}

private void restore()
{
	mapping map;
	string buf;

	muds = ([ ]);
	mudlistid = 0;

	channels = ([ ]);
	chanlistid = 0;

	password = 0;

	buf = SECRETD->read_file("intermud");

	if (buf) {
		catch {
			map = PARSER_VALUE->parse(buf);

			if (map && map["password"]) {
				password = map["password"];
			}

			if (map && map["routers"]) {
				routers = map["routers"];
			}

			if (map && map["router"]) {
				router = map["router"];
			}
		} : {
			LOGD->post_message("system", LOG_ERR, "IntermudD: Error parsing Intermud state, resetting");
			SECRETD->remove_file("intermud-bad");
			SECRETD->rename_file("intermud", "intermud-bad");
		}
	}

	if (!routers) {
		call_out("reset_routers", 0);
	}

	if (!router) {
		router = "*wpr";
		call_out("save", 0);
	}
}

void add_router(string name, string ip, int port)
{
	ACCESS_CHECK(VERB());

	if (!routers) {
		routers = ([ ]);
	}

	routers[name] = ({ ip, port });

	call_out("save", 0);
}

void remove_router(string name)
{
	ACCESS_CHECK(VERB());

	if (!routers) {
		routers = ([ ]);
	}

	routers[name] = nil;

	call_out("save", 0);
}

string *query_routers()
{
	return map_indices(routers);
}

mixed *query_router(string name)
{
	return routers[name];
}

void reset_routers()
{
	routers = ([
		"*dalet": ({ "97.107.133.86", 8787 }),
		"*Kelly": ({ "150.101.219.57", 8080 }),
		"*i4": ({ "204.209.44.3", 8080 }),
		"*wpr": ({ "136.144.155.250", 8080 })
	]);

	router = "*wpr";

	call_out("save", 0);
}
