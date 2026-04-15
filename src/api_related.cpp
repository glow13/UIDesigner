#include <geode.devtools/include/API.hpp>


void example() {
	int current = 0;
	devtools::combo("Example Combo", current, { "Item 1", "Item 2", "Item 3" }, 5);
	devtools::newLine();
	devtools::sameLine();
	devtools::nextItemWidth(10);
	devtools::separator();

	std::string a;
	devtools::inputMultiline("a", a);

	int b;
	devtools::radio("hi", b, b);
}
