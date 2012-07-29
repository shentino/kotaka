#include <kotaka/paths.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <status.h>

object maze;
int goal_size;
int adjust;

static void adjust();

void create()
{
	maze = clone_object(BIGSTRUCT_ARRAY_OBJ);
	goal_size = 1;
	adjust();
	call_out("spawn", 1.0);
}

void cpr()
{
	mixed **callouts;
	int sz, i;

	callouts = status(this_object(), O_CALLOUTS);
	sz = sizeof(callouts);

	for (i = 0; i < sz; i++) {
		remove_call_out(callouts[i][CO_HANDLE]);
	}

	adjust = call_out("adjust", 0);
	call_out("spawn", 0);
}

static void spawn()
{
	clone_object("~/obj/runner");

	call_out("spawn", 0.1);
}

void destruct()
{
	object basement;

	if (maze->get_size() > 1) {
		error("Shrink me first (" + maze->get_size() + ")");
	}

	if (maze->get_size() && (basement = maze->get_element(0)))
		destruct_object(basement);
	destruct_object(maze);
}

void set_size_goal(int new_goal)
{
	if (new_goal < 1) {
		error("Invalid goal size");
	}

	goal_size = new_goal;

	if (!adjust) {
		adjust = call_out("adjust", 0);
	}
}

static void adjust()
{
	int sz;
	adjust = 0;

	sz = maze->get_size();

	if (sz > goal_size) {
		object turkey;

		adjust = call_out("adjust", 0);

		turkey = maze->get_element(sz - 1);
		maze->pop_back();

		if (turkey) {
			object basement;

			basement = maze->get_element(0);

			turkey->move(basement);
			destruct_object(turkey);
			ASSERT(!turkey);
		} else {
			LOGD->post_message("maze", LOG_DEBUG, "Empty turkey");
		}
	} else if (sz < goal_size) {
		object newroom;
		adjust = call_out("adjust", 0);

		newroom = clone_object("~/obj/room");
		maze->push_back(newroom);
		ASSERT(maze->get_element(sz) == newroom);
	}
}

void reset()
{
	destruct_object(maze);
	maze = clone_object(BIGSTRUCT_ARRAY_OBJ);
	goal_size = 1;
	adjust();
}

int query_size()
{
	return maze->get_size();
}

object query_room(int roomnum)
{
	return maze->get_element(roomnum);
}
