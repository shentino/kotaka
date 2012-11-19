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
#include <kotaka/paths.h>
#include <type.h>
#include <trace.h>

/* call_other masking is optional */
#if 1

/* but if we have call_other, the other routines are mandatory */
#if 1
static mixed call_other(mixed obj, string func, varargs mixed args ...)
{
	string path;

	if (!this_object()) {
		error("Cannot call_other from destructed object");
	}

	switch(typeof(obj)) {
	case T_STRING:
		path = obj;
		obj = find_object(path);
		break;

	case T_OBJECT:
		path = object_name(obj);
		break;

	default:
		error("Bad argument 1 for function call_other (type mismatch)");
	}

	if (!obj) {
		error("Bad argument 1 for function call_other (target object " + path + " does not exist)");
	}

	if (!function_object(func, obj)) {
		error("Call to undefined function " + func + " in object " + path);
	}

	return ::call_other(obj, func, args ...);
}
#endif

static string previous_program(varargs int n)
{
	int idx;
	string tmp;

	idx = 1;

	while (1) {
		tmp = ::previous_program(idx);

		if (tmp != CALL_GUARD) {
			if (!n) {
				return tmp;
			}
			n--;
		}
		idx++;
	}
}

static mixed **call_trace()
{
	mixed **trace;
	int index;
	int sz;

	trace = ::call_trace();
	sz = sizeof(trace);

	for (index = 0; index < sz; index++) {
		mixed *frame;

		frame = trace[index];

		if (frame[TRACE_PROGNAME] == CALL_GUARD) {
			trace[index] = nil;
		}
	}

	return trace - ({ nil });
}
#endif

static mixed call_other_unprotected(varargs mixed args ...)
{
	return ::call_other(args ...);
}

static mixed **call_trace_unprotected()
{
	return ::call_trace();
}

static string previous_program_unprotected(varargs int steps)
{
	return ::previous_program(steps);
}

static object calling_object(varargs int steps)
{
	mixed **trace;

	trace = call_trace();

	return find_object(trace[sizeof(trace) - (3 + steps)][TRACE_OBJNAME]);
}
