#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

string path;

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
	send_out("[\033[1;35mHelp\033[0m] ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out("Welcome to the help system.\n\n");
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

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "quit":
		pop_state();
		return;
	default:
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
