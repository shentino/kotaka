inherit "support";
inherit "/lib/string/sprint";

private string detail_box(object obj, string detail)
{
	string buffer;
	string *descriptions;
	string *words;
	int sz;

	buffer = "<p>";

	if (sizeof(words = obj->query_snouns(detail))) {
		buffer += "Singular nouns: " + implode(words, ", ") + "<br />\n";
	}

	if (sizeof(words = obj->query_pnouns(detail))) {
		buffer += "Plural nouns: " + implode(words, ", ") + "<br />\n";
	}

	if (sizeof(words = obj->query_adjectives(detail))) {
		buffer += "Adjectives: " + implode(words, ", ") + "<br />\n";
	}

	descriptions = obj->query_descriptions(detail);
	sz = sizeof(descriptions);

	if (sz) {
		int i;

		buffer += "</p>\n<p>";

		for (i = 0; i < sz; i++) {
			string description;

			description = descriptions[i];

			buffer += description + ": " + obj->query_description(detail, description) + "<br />\n";
		}

		buffer += "</p>\n";
	}

	if (detail) {
		return oinfobox(detail, 3, buffer);
	} else {
		return oinfobox("(default)", 3, buffer);
	}
}

static string detail_text(object obj, string *details)
{
	string buffer;
	int sz, i;

	sz = sizeof(details);

	buffer = "<h2>Details</h2>\n";

	for (i = 0; i < sz; i++) {
		buffer += detail_box(obj, details[i]);
	}

	return buffer;
}
