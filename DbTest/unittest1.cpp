#include "stdafx.h"

#define CATCH_CONFIG_RUNNER // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

static bool init = true;

int main(int argc, char ** argv)
{
	bool pause_after_test = true;
	if (argc && std::string(argv[argc - 1]) == "-p")
	{
		pause_after_test = true;
		argc--;
	}

	int result = Catch::Session().run(argc, argv);

	if (pause_after_test)
	{
		printf("Press enter to continue.");
		std::string s;
		std::cin >> s;
	}

	return result;
}

TEST_CASE("Database Insert And Retrieve"){
	/*
		gets called x amount of times depending on number of sections
		basically the Setup()
	*/
	signalpp::Storage<signalpp::NuDB> storage("testdb");
	SECTION("Database Insert And Retrieve String") {
		std::string number = "+31622222222";
		storage.put("number", number);
		std::string value;
		storage.get("number", value);
		REQUIRE(strcmp(value.c_str(), number.c_str()) == 0);
	}
	SECTION("Database Insert And Retrieve Integer") {
		int divider = 42;
		storage.put("divider", divider);
		int value;
		storage.get("divider", value);
		REQUIRE(divider == value);
	}
	// teardown
	storage.close();
}