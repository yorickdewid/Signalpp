#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <chrono>

#include <signalpp/signalpp.h>
#include "Json.h"

#define CLIENT_NAME		"SignalClient++"

std::shared_ptr<signalpp::StorageContainer> storage = std::make_shared<signalpp::Storage<signalpp::UnqliteDB>>("db");
bool firstRun = false;

void init(bool firstRun = false) {
	if (!signalpp::Registration::isDone(storage)) {
		return;
	}

	std::cout << "Starting init" << std::endl;

	std::string username, password, signalingKey;
	storage->get("number_id", username);
	storage->get("password", password);
	storage->get("signaling_key", signalingKey);

	std::cout << "username: " << username << std::endl;
	std::cout << "password: " << password << std::endl;

	/* Initialize the socket and start listening for messages */
	signalpp::MessageReceiver messageReceiver(storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);

	/* Actions on incomming messages */
	messageReceiver.onMmessage([] {});
	messageReceiver.onReceipt([] {});
	messageReceiver.onContact([] {});
	messageReceiver.onGroup([] {});
	messageReceiver.onSent([] {});
	messageReceiver.onRead([] {});
	messageReceiver.onError([] {});

	signalpp::MessageSender messageSender(storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);

	// if (firstRun) {
	int device_id = 0;
	if (!storage->get("device_id", device_id))
		return;
	if (!device_id || device_id == 1)
		return;

	std::cout << "First run" << std::endl;

	signalpp::SyncRequest syncRequest(messageSender, messageReceiver);

	syncRequest.onSuccess([&syncRequest]() {
		std::cout << "Sync successful" << std::endl;
		storage->put("synced_at", signalpp::getTimestamp());
		syncRequest.onContactSyncComplete();
	});

	syncRequest.onTimeout([&syncRequest]() {
		std::cout << "Sync timed out" << std::endl;
		syncRequest.onContactSyncComplete();
	});
}

std::string getPhoneNumberFromConfig() {
	std::ifstream myfile;
	/* see config.template for creating config.json*/
	myfile.open("..\\config.json", std::ios::in);
	std::string line;
	std::ostringstream json_serialized;
	if (myfile.is_open()) {
		while (std::getline(myfile, line))
		{
			json_serialized << line;
		}
		myfile.close();
	}
	nlohmann::json json = nlohmann::json::parse(json_serialized.str());
	return json.at("sendToPhoneNumber");
}


void interactive_client() {
	std::string toPhoneNumber = getPhoneNumberFromConfig();
	std::string username, self_number, password, signalingKey;
	storage->get("number", self_number);
	storage->get("number_id", username);
	storage->get("password", password);
	storage->get("signaling_key", signalingKey);
	int deviceId = -1;
	storage->get("device_id", deviceId);
	signalpp::MessageSender messageSender(storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);
	bool interactive = true;
	while (interactive) {
		std::cout << "Enter string:";
		std::string* body = nullptr;
		std::string s;
		std::getline(std::cin, s);
		if (s.compare("/exit()") == 0) {
			interactive = false;
		}
		body = new std::string(s);
		messageSender.sendMessage(self_number, deviceId, body, toPhoneNumber);
		std::cout << "Sending message: " << s << std::endl;
	}
}

void register_client() {
	std::string username, password;
	storage->get("username", username);
	storage->get("password", password);

	signalpp::AccountManager accountManager(storage, signalpp::serverUrl, signalpp::serverPorts, username, password);

	/* For the moment just show the URL */
	auto provisionUrl = [](const std::string& url) {
		std::cout << "Provision URL: " << url << std::endl;
	};

	/* For the moment just show the URL */
	auto confirmNumber = [](const std::string& number) -> std::string {
		std::cout << "Number: " << number << std::endl;
		return CLIENT_NAME;
	};

	if (accountManager.registerSecondDevice(provisionUrl, confirmNumber)) {
		signalpp::Registration::markDone(storage);
	}

	firstRun = true;
}

int main(int argc, char *argv[]) {
	namespace cli = boost::program_options;

	cli::variables_map vm;
	cli::options_description desc("Usage: signalclient [OPTIONS]");
	desc.add_options()
		("help", "Print help messages")
		("add", "additional options")
		("purge", "Purge current client data")
		("version", "Library version and quit");

	try
	{
		cli::store(cli::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help"))
		{
			std::cout << CLIENT_NAME " - Commandline Signal client" << std::endl << std::endl << desc << std::endl;
			return 1;
		}

		if (vm.count("version"))
		{
			std::cout << "Version: " << signalpp::getVersion() << std::endl;
			return 0;
		}

		cli::notify(vm);
	}
	catch (cli::error& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		std::cerr << desc << std::endl;
		return 1;
	}


	/* Initialize environment */
	signalpp::Env env(storage); //weirdly enough this branches off accountmanager

	/* Register client if this first run */
	if (!signalpp::Registration::everDone(storage)) {
		register_client();
	}

	std::string IdentityKey;
	/* Key Not found, both NuDB and Unqlite db have this problem, so it's our code somehow.
		Relative db path was not the issue.
		Uncommited values also was not.
	*/
	storage->commit();

	/* Initialize the client */
	init(firstRun);

	interactive_client();

	return 0;
}