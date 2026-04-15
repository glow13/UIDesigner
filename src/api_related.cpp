#include <geode.devtools/include/API.hpp>


void example() {
	const char* values[] = { "Item 1", "Item 2", "Item 3" };
	int current = 0;
	devtools::combo("Example Combo", current, values, 5);
	devtools::newLine();
	devtools::sameLine();
	devtools::nextItemWidth(10);
	devtools::separator();

	std::string a;
	devtools::inputMultiline("a", a);

	devtools::label("label");
	devtools::button("button");

	int b = 0;
	devtools::radio("hi", b, b);
}
