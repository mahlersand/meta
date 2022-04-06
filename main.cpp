#include <iostream>
#include <memory>

#include <meta/object>

using std::string;
using std::cerr;
using std::endl;

using std::shared_ptr;

using meta::signal;
using meta::slot;
using meta::connection;
using meta::property;

class Example : public meta::object<Example>
{
public:
	property<int> windowSizeX;

	Example() : meta::object<Example>()
	{

	}

protected:
	virtual void properties()
	{

	}
};


int main()
{
	property<std::string> prop_a("Mango");
	property<std::string> prop_b("Banane");

	connect(prop_a.changed,
	        prop_a.set);

	cerr << string(prop_a) << endl
	     << string(prop_b) << endl;

	property<string> s = prop_a = "Keks";

	cerr << string(prop_a) << endl
	     << string(prop_b) << endl;

	return 0;
}
