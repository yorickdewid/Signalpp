#include <iostream>
#include <sstream>
#include <string>

#include <signal/signal.h>

signal::Storage<signal::Ldb> storage;

void registerClient() {
	std::string username = storage.get("username");
	std::string password = storage.get("password");

	signal::AccountManager accountManager(signal::serverUrl, signal::serverPorts, username, password);

	auto provisionUrl = [] (const std::string& url) {
		SIGNAL_LOG_INFO << "Provision URL: " << url;
	};

	auto confirmNumber = [] (const std::string& number) {
		SIGNAL_LOG_INFO << "Number: " << number;
	};

	if (accountManager.registerSecondDevice(provisionUrl, confirmNumber)) {
		signal::Registration::markDone(storage);
	}

	// textsecure:contactsync
}

int main(int argc, char *argv[]) {
	signal::Logger::setLogLevel(signal::LogLevel::DEBUG);

	SIGNAL_LOG_INFO << "Library version " << signal::getVersion();

	if (!signal::Registration::everDone(storage)) {
		registerClient();
	}

	// openInbox();

	return 0;
}
