#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;

void reboot()
{
	ACCESS_CHECK(previous_program() == INITD);
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == INITD);
}

void dumped_state()
{
	ACCESS_CHECK(previous_program() == INITD);
}

int forbid_inherit(string from, string path, int priv)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return 0;
}

int forbid_call(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return 0;
}

string include_file(string compiled, string from, string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return path;
}

string query_constructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return nil;
}

string query_destructor(string path)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return nil;
}
