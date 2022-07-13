#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <iostream>
#include <memory>
#include <cxxabi.h>
#include <thread>

#include <meta/object>

using std::string;
using std::cerr;
using std::endl;

using std::shared_ptr;
using std::get;


TEST_CASE("Test signal creation and deletion")
{
	meta::signal<std::string> s;
}

TEST_CASE("Test slot creation and deletion")
{
	meta::slot<std::string> s;
}

TEST_CASE("Test signal/slot connection with dropping")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	signal<string, int, int, float> sender;
	slot<string, int, float> receiver;

	receiver.setCallable([](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
	});

	connect(sender, receiver);

	sender("Banana", 10, 26, -5.0);
}
