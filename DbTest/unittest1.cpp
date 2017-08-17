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

template <class T,typename V>
V put_and_verify(signalpp::Storage<T>& storage, const std::string& key, const V& expected) {
	storage.put(key, expected);
	V value;
	storage.get(key, value);
	return value;
}

void for_each(signalpp::Storage<signalpp::NuDB>& storage)
{
	//auto const appnum = db_.appnum();
	nudb::error_code ec;
	storage.close();

	nudb::visit("testdb.dat",
		[&](
			void const* key, std::size_t key_bytes,
			void const* data, std::size_t size,
			nudb::error_code&)
	{

		std::string key_s((char const*)key, key_bytes);
		std::string ret((char const*)data, size);
		nudb::detail::buffer bf;
		printf_s(("Key: " + key_s + "\n").c_str());
		printf_s(("Data: " + ret + "\n").c_str());

	}, nudb::no_progress{}, ec);
	//storage.open(dp, kp, lp, ec);
}

TEST_CASE("UnqliteDB tests") {
	signalpp::Storage<signalpp::UnqliteDB> storage("testdb");
	SECTION("Database Insert And Retrieve Integer") {
		int expected = 42;
		int value = put_and_verify<signalpp::UnqliteDB, int>(storage, "divider", expected);
		REQUIRE(value == expected);
	}
	SECTION("identityKey") {
		std::string expected = "\x5Z­ñEaâ:{T?ƒ^îÊ\aÔ\\õ¤T\bcrçÈ‘Ì‰\x6\x6V$&&$à\tŞ6ºæ;5\t,Â\x13\x16«aÔ\xfe´×\"™+É \f¢#j";
		std::string value = put_and_verify<signalpp::UnqliteDB,std::string>(storage, "identityKey", expected);
		//printf_s(value.c_str());
		//printf_s(expected.c_str());
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	storage.close();
}

TEST_CASE("NuDB Database Insert And Retrieve") {
	/*
		gets called x amount of times depending on number of sections
		basically the Setup()
	*/


	signalpp::Storage<signalpp::NuDB> storage("testdb");
	//std::string identitykey = "IdentityKey";
	//SECTION("Insert IdentityKey") {
	//	std::string expected = "\x5Z­ñEaâ:{T?ƒ^îÊ\aÔ\\õ¤T\bcrçÈ‘Ì‰\x6\x6V$&&$à\tŞ6ºæ;5\t,Â\x13\x16«aÔ\xfe´×\"™+É \f¢#j";
	//	storage.put(identitykey, expected);
	//	REQUIRE(true == true);
	//}
	//SECTION("Retrieve IdentityKey") {
	//	std::string expected = "\x5Z­ñEaâ:{T?ƒ^îÊ\aÔ\\õ¤T\bcrçÈ‘Ì‰\x6\x6V$&&$à\tŞ6ºæ;5\t,Â\x13\x16«aÔ\xfe´×\"™+É \f¢#j";
	//	std::string retrieved;
	//	bool result = storage.get(identitykey, retrieved);
	//	REQUIRE(strcmp(retrieved.c_str(), expected.c_str()) == 0);
	//}
	SECTION("Database Insert And Retrieve Integer") {
		int expected = 42;
		int value = put_and_verify<signalpp::NuDB,int>(storage, "divider", expected);
		REQUIRE(value == expected);
	}
	SECTION("identityKey") {
		std::string expected = "\x5Z­ñEaâ:{T?ƒ^îÊ\aÔ\\õ¤T\bcrçÈ‘Ì‰\x6\x6V$&&$à\tŞ6ºæ;5\t,Â\x13\x16«aÔ\xfe´×\"™+É \f¢#j";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "identityKey", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("signalingKey") {
		std::string expected = "&’¶øÅÉ9ëı–\x12x[À¬ìÂë1A° \x1b¬M\x1¢©C¦é¨ÕÕÑ¼\x15™Ä÷¬\nÕ\r HÖ’ïÛ";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "signalingKey", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("password") {
		std::string expected = "mSx/ayeZOe3AOKohzG+wrA";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "password", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("registrationId") {
		std::string expected = "12800";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "registrationId", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("number") {
		std::string expected = "+31622222222";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "number", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("number_id") {
		std::string expected = "+31622222222.2";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "number_id", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("deviceName") {
		std::string expected = "SignalClient++";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "deviceName", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("deviceId") {
		std::string expected = "2";
		std::string value = put_and_verify<signalpp::NuDB, std::string>(storage, "deviceId", expected);
		REQUIRE(strcmp(value.c_str(), expected.c_str()) == 0);
	}
	SECTION("EVAL") {
		for_each(storage);
	}
	storage.close();


	// teardown
	//storage.close();
}