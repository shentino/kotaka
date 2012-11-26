/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>

#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

string titled_name(string name, int class)
{
	string username;

	username = name;

	if (name) {
		name = STRINGD->to_title(name);
	} else {
		name = "guest";
	}

	switch(class) {
	case 0:
		name = "\033[1;34m" + name + "\033[0m";
		break;
	case 1:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mMr. " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mMs. " + name + "\033[0m";
			break;
		}
		break;
	case 2:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mSir " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mDame " + name + "\033[0m";
			break;
		}
		break;
	case 3:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mLord " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mLady " + name + "\033[0m";
			break;
		}
		break;
	}

	return name;
}

void send_to_all(string phrase)
{
	int sz;
	object *users;

	ACCESS_CHECK(TEXT());

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	ACCESS_CHECK(TEXT());

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();
	users -= exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

int query_user_class(string username)
{
	if (!username) {
		return 0;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		return 0;
	}

	if (KERNELD->access(username, "/", FULL_ACCESS)) {
		return 3;
	}

	if (sizeof( KERNELD->query_users() & ({ username }) )) {
		return 2;
	}

	return 1;
}

object mega_inventory(object root)
{
	object obj;
	object *inv;
	int sz, i;
	object list;
	object queue;

	list = new_object(BIGSTRUCT_ARRAY_LWO);
	list->grant_access(previous_object(), FULL_ACCESS);
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
	queue->push_back(root);

	while (!queue->empty()) {
		object obj;

		obj = queue->get_front();
		queue->pop_front();

		list->push_back(obj);

		inv = obj->query_inventory();
		sz = sizeof(inv);

		for (i = 0; i < sz; i++) {
			queue->push_back(inv[i]);
		}
	}

	return list;
}
