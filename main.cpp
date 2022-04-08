#include <iostream>
#include <memory>
#include <cxxabi.h>

#include <meta/object>

using std::string;
using std::cerr;
using std::endl;

using std::shared_ptr;
using std::get;

using meta::signal;
using meta::slot;
using meta::connection;
using meta::property;

int add(int a, int b)
{
	return a + b;
}

int main()
{
	int x = meta::detail::apply_drop<int, int>(&add, std::make_tuple('A',
	                                                                 8,
	                                                                 std::string("Hallo Welt"),
	                                                                 24));

	cerr << x << endl;

	return 0;
}
