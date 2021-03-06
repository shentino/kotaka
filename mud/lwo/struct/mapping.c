/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <type.h>
#include <kotaka/assert.h>

inherit "~System/lib/struct/list";
inherit "~System/lib/search";

int branch;
int leaf;

mixed *root;
int type;

/* node structure: */

/* leaf node */
/* ({ size, ([ key : value ]) }) */

/* branch node */
/* ({ keys, subnodes }) */

private mixed split_node(mixed *parent, int chubby)
{
	mixed *keys;
	mixed **nodes;
	mixed *node;
	mixed splitkey;
	mixed *splitnode;
	mixed head;
	int hsz;

	keys = parent[0];
	nodes = parent[1];
	node = nodes[chubby];

	head = node[0];

	switch (typeof(head)) {
	case T_INT: /* splitting a leaf node */
		{
			mixed *subkeys;
			mapping submap;
			mapping lower, upper;

			submap = node[1];
			subkeys = map_indices(submap);

			splitkey = subkeys[sizeof(subkeys) / 2];

			lower = submap[.. splitkey];
			lower[splitkey] = nil;

			upper = submap[splitkey ..];

			node[0] = map_sizeof(lower);
			node[1] = lower;

			splitnode = ({ map_sizeof(upper), upper });
		}
		break;

	case T_ARRAY: /* splitting a branch node */
		{
			mixed *subkeys;
			mixed **subnodes;
			int hsz;

			subkeys = head;
			subnodes = node[1];

			hsz = sizeof(subkeys) / 2;

			splitkey = subkeys[hsz];

			node[0] = subkeys[.. hsz - 1];
			node[1] = subnodes[.. hsz - 1];

			splitnode = ({ subkeys[hsz ..], subnodes[hsz ..] });
		}
		break;
	}

	if (chubby == sizeof(keys)) {
		keys += ({ splitkey });
		nodes += ({ splitnode });
	} else {
		keys = keys[.. chubby] + ({ splitkey }) + keys[chubby + 1 ..];
		nodes = nodes[.. chubby] + ({ splitnode }) + nodes[chubby + 1 ..];
	}

	parent[0] = keys;
	parent[1] = nodes;

	return splitkey;
}

private int sub_set_element(mixed *node, mixed key, mixed value)
{
	mixed head;

	head = node[0];

	switch(typeof(head)) {
	case T_INT: /* leaf node */
		{
			mapping map;

			map = node[1];

			if (value != nil) {
				if (head >= leaf) {
					/* compact */
					head = map_sizeof(node[1]);

					if (head >= leaf) {
						/* overflow */
						return -1;
					}
				}
			}

			if (map[key] != nil) {
				head--;
			}

			map[key] = value;

			if (map[key] != nil) {
				head++;
			}

			node[0] = head;
		}
		break;

	case T_ARRAY: /* branch node */
		{
			int subindex;
			mixed subkey;
			mixed splitkey;
			mixed newkey;
			mixed *subnode;

			/* find floor */
			subindex = binary_search_floor(head, key);

			if (subindex == -1) {
				subindex = 0;
				newkey = key;
			}

			subkey = head[subindex];
			subnode = node[1][subindex];

			if (sub_set_element(subnode, key, value) == -1) {
				if (sizeof(head) > branch) {
					/* we're too big to be split, punt */
					return -1;
				}

				splitkey = split_node(node, subindex);

				if (key >= splitkey) {
					/* upper half */
					subnode = node[1][subindex + 1];
				}

				sub_set_element(subnode, key, value);
			}

			if (newkey != nil) {
				node[0][0] = newkey;
			}

			return 0;
		}
	}
}

private mixed sub_query_element(mixed *node, mixed key)
{
	mixed head;

	head = node[0];

	switch(typeof(head)) {
	case T_INT:
		return node[1][key];

	case T_ARRAY:
		{
			int subindex;
			mixed subkey;
			mixed subnode;

			/* find floor */
			subindex = binary_search_floor(head, key);

			if (subindex == -1) {
				return nil;
			}

			return sub_query_element(node[1][subindex], key);
		}
	}
}

private void sub_query_indices(mixed *node, mixed **list)
{
	mixed head;

	head = node[0];

	switch(typeof(head)) {
	case T_INT:
		{
			int i, sz;

			head = map_indices(node[1]);
			sz = sizeof(head);

			for (i = 0; i < sz; i++) {
				list_push_back(list, head[i]);
			}
		}
		break;

	case T_ARRAY:
		{
			int i, sz;

			sz = sizeof(head);

			for (i = 0; i < sz; i++) {
				sub_query_indices(node[1][i], list);
			}
		}
	}
}

void set_leaf_limit(int new_leaf)
{
	leaf = new_leaf;
}

void set_branch_limit(int new_branch)
{
	branch = new_branch;
}

void set_type(int new_type)
{
	switch(new_type)
	{
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		type = new_type;
		root = ({ 0, ([ ]) });
		return;

	default:
		error("Invalid type");
	}
}

void set_element(mixed key, mixed value)
{
	if (typeof(key) != type) {
		error("Type mismatch");
	}

	if (!leaf) {
		leaf = 256;
	}

	if (!branch) {
		branch = 16;
	}

	if (sub_set_element(root, key, value) == -1) {
		mixed head;
		mixed bottom;

		head = root[0];

		switch(typeof(head)) {
		case T_INT:
			head = map_indices(root[1]);
			break;

		case T_ARRAY:
			break;
		}

		bottom = head[0];

		root = ({
			({ bottom }),
			({ root })
		});

		sub_set_element(root, key, value);
	}
}

mixed query_element(mixed key)
{
	return sub_query_element(root, key);
}

mixed *query_root()
{
	return root;
}

mixed **query_indices()
{
	mixed **list;

	list = ({ nil, nil });

	sub_query_indices(root, list);

	return list;
}
