#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>

#include <signalpp/signalpp.h>

#define CLIENT_NAME		"SignalClient++"

signalpp::Storage<signalpp::NuDB> storage("db");
bool firstRun = false;

void init(bool firstRun = false) {
	if (!signalpp::Registration::isDone(storage)) {
		return;
	}

	std::cout << "Starting init" << std::endl;

	std::string username, password, signalingKey;
	storage.get("number_id", username); // key not found error.
	storage.get("password", password);
	storage.get("signaling_key", signalingKey);

	std::cout << "username: " << username << std::endl;
	std::cout << "password: " << password << std::endl;

	/* Initialize the socket and start listening for messages */
	signalpp::MessageReceiver messageReceiver(&storage,
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

	signalpp::MessageSender messageSender(&storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);

	// if (firstRun) {
	int device_id = 0;
	if (!storage.get("device_id", device_id))
		return;
	if (!device_id || device_id == 1)
		return;

	std::cout << "First run" << std::endl;

	signalpp::SyncRequest syncRequest(messageSender, messageReceiver);

	syncRequest.onSuccess([&syncRequest]() {
		std::cout << "Sync successful" << std::endl;
		storage.put("synced_at", signalpp::getTimestamp());
		syncRequest.onContactSyncComplete();
	});

	syncRequest.onTimeout([&syncRequest]() {
		std::cout << "Sync timed out" << std::endl;
		syncRequest.onContactSyncComplete();
	});
	// }
}

void register_client() {
	std::string username, password;
	storage.get("username", username);
	storage.get("password", password);

	signalpp::AccountManager accountManager(&storage, signalpp::serverUrl, signalpp::serverPorts, username, password);

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

void for_each(signalpp::Storage<signalpp::NuDB>& storage)
{
	//auto const appnum = db_.appnum();
	nudb::error_code ec;
	storage.close();

	nudb::visit("db.dat",
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
	signalpp::Env env(&storage);

	/* Register client if this first run */
	if (!signalpp::Registration::everDone(storage)) {
		register_client();
	}

	for_each(storage);
	storage = signalpp::Storage<signalpp::NuDB>("db");
	//storage.initialize("db");
	std::string IdentityKey;
	//std::string SignalingKey;
	//std::string PasswordKey;
	//std::string RegistrationKey;
	//std::string NumberKey;
	//std::string NumberIdKey;
	//std::string DevicenameKey;
	//std::string DeviceIdKey;
	storage.get("identityKey", IdentityKey);
	////printf_s(serialize.c_str());
	//storage.get("signaling_key", SignalingKey); // works
	//storage.get("password", PasswordKey); // breaks?
	//storage.get("registrationId", RegistrationKey);
	//storage.get("number", NumberKey);
	//storage.get("number_id", NumberIdKey);
	//storage.get("device_name", DevicenameKey);
	//storage.get("device_id", DeviceIdKey);

	/* Initialize the client */
	init(firstRun);

	getchar();

	return 0;
}