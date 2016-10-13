#include <iostream>
#include <sstream>
#include <string>

#include <signal/signal.h>

signal::Storage<signal::Ldb> storage;

void registerClient() {
	std::string username = storage.get("username");
	std::string password = storage.get("password");

	signal::AccountManager accountManager(signal::serverUrl, signal::serverPorts, username, password);

	auto provisionUrl = [](const std::string& url) {
		std::cout << url << std::endl;
	};

	accountManager.registerSecondDevice(
		provisionUrl,
		[]{}
	);
    
    // textsecure:contactsync

    signal::Registration::markDone(storage);
}

int main(int argc, char *argv[]) {
	std::cout << "Running " << signal::getVersion() << std::endl;

	if (!signal::Registration::everDone(storage)) {
		registerClient();
	}

	// openInbox();

	return 0;
}
