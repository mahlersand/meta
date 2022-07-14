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

TEST_CASE("Stack signals/slots")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	signal<string, int, int, float> sender;
	slot<string, int, float> receiver;

	bool called = false;

	receiver.setCallable([&called](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		called = true;
	});

	auto c = connect(sender, receiver);

	sender("Banana", 10, 26, -5.0);

	REQUIRE(called);
}

TEST_CASE("Heap signals/slots")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	auto sender = std::make_shared<signal<string, int, int, float>>();
	auto receiver = std::make_shared<slot<string, int, float>>();

	bool called = false;

	receiver->setCallable([&called](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		called = true;
	});

	auto c = connect(*sender, *receiver);

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(called);
}

TEST_CASE("Ad hoc connections")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	bool called = false;

	auto sender = std::make_shared<signal<string, int, int, float>>();
	auto receiver = [&called](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		called = true;
	};

	auto c = connect(*sender, std::function(receiver));

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(called);
}

TEST_CASE("Disconnection")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	auto sender = std::make_shared<signal<string, int, int, float>>();
	auto receiver = std::make_shared<slot<string, int, float>>();

	int calls = 0;

	receiver->setCallable([&calls](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		++calls;
	});

	auto c = connect(*sender, *receiver);

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);

	c->disconnect();

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);
}

TEST_CASE("Disconnection on deletion")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	auto sender = std::make_shared<signal<string, int, int, float>>();
	auto receiver = std::make_shared<slot<string, int, float>>();

	int calls = 0;

	receiver->setCallable([&calls](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		++calls;
	});

	auto c = connect(*sender, *receiver);

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);

	receiver.reset();

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);
}

TEST_CASE("Disconnection of ad hoc connections")
{
	using namespace std;
	using meta::signal;
	using meta::slot;

	int calls = 0;

	auto sender = std::make_shared<signal<string, int, int, float>>();
	auto receiver = [&calls](string s, int i, float f){
		REQUIRE(s == "Banana");
		REQUIRE(i == 10);
		REQUIRE(f == float(26));
		++calls;
	};

	auto c = connect(*sender, std::function(receiver));

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);

	c->disconnect();

	(*sender)("Banana", 10, 26, -5.0);

	REQUIRE(calls == 1);
}
