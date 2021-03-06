#include <iostream>
#include <sstream>
#include <string>

#include <getopt.h>

#include <signalpp/signalpp.h>

#define CLIENT_NAME		"SignalClient++"

signalpp::Storage<signalpp::Ldb> storage;
bool firstRun = false;

void init(bool firstRun = false) {
	if (!signalpp::Registration::isDone(storage)) {
		return;
	}

	SIGNAL_LOG_INFO << "Starting init";

	std::string username = storage.get("number_id");
	std::string password = storage.get("password");
	std::string signalingKey = storage.get("signaling_key");

	SIGNAL_LOG_DEBUG << "username: " << username;
	SIGNAL_LOG_DEBUG << "password: " << password;

	/* Initialize the socket and start listening for messages */
	signalpp::MessageReceiver messageReceiver(&storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);

	/* Actions on incomming messages */
	messageReceiver.onMmessage([]{});
	messageReceiver.onReceipt([]{});
	messageReceiver.onContact([]{});
	messageReceiver.onGroup([]{});
	messageReceiver.onSent([]{});
	messageReceiver.onRead([]{});
	messageReceiver.onError([]{});

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

		SIGNAL_LOG_INFO << "First run";

		signalpp::SyncRequest syncRequest(messageSender, messageReceiver);
		
		syncRequest.onSuccess([&syncRequest] () {
			SIGNAL_LOG_INFO << "Sync successful";
			storage.put("synced_at", signalpp::getTimestamp());
			syncRequest.onContactSyncComplete();
		});

		syncRequest.onTimeout([&syncRequest] () {
			SIGNAL_LOG_INFO << "Sync timed out";
			syncRequest.onContactSyncComplete();
		});
	// }
}

void register_client() {
	std::string username = storage.get("username");
	std::string password = storage.get("password");

	signalpp::AccountManager accountManager(&storage, signalpp::serverUrl, signalpp::serverPorts, username, password);

	/* For the moment just show the URL */
	auto provisionUrl = [] (const std::string& url) {
		SIGNAL_LOG_INFO << "Provision URL: " << url;
	};

	/* For the moment just show the URL */
	auto confirmNumber = [] (const std::string& number) -> std::string {
		SIGNAL_LOG_INFO << "Number: " << number;
		return CLIENT_NAME;
	};

	if (accountManager.registerSecondDevice(provisionUrl, confirmNumber)) {
		signalpp::Registration::markDone(storage);
	}

	firstRun = true;
}

void usage(const char *prog) {
	printf("Usage: %s [OPTIONS]\n\n", prog);
	printf(" -h  --help             Show this help\n");
	printf(" -l  --list-devices     List all registered devices\n");
	printf(" -S  --secure           Enable additional security\n");
	printf(" -p  --profile          Show current profile\n");
	printf("     --sms NUMBER       Send an SMS verification code\n");
	printf("     --voice NUMBER     Send voice verification code\n");
	printf("     --purge            Purge current client data\n");
	printf(" -V  --verbose          More verbose output\n");
	printf(" -v  --version          Library version and quit\n");
}

int main(int argc, char *argv[]) {
	int opt= 0;

	static struct option long_options[] = {
		{"list-devices",   no_argument,       0,  'l' },
		{"profile",        no_argument,       0,  'p' },
		{"sms",            required_argument, 0,  'S' },
		{"voice",          required_argument, 0,  'O' },
		{"purge",          no_argument,       0,  'P' },
		{"secure",         no_argument,       0,  's' },
		{"verbose",        no_argument,       0,  'V' },
		{"help",           no_argument,       0,  'h' },
		{"version",        no_argument,       0,  'v' },
		{0,                0,                 0,   0  }
	};

	int long_index = 0;
	while ((opt = getopt_long(argc, argv, "lpvhV", long_options, &long_index )) != -1) {
		switch (opt) {
			case 'l' :
				break;
			case 'p' :
				break;
			case 'S' :
				// optarg
				break;
			case 'O' :
				// optarg
				break;
			case 'P' :
				signalpp::Env::purge(storage);
				break;
			case 's' :
				break;
			case 'V' :
				signalpp::Logger::setLogLevel(signalpp::LogLevel::DEBUG);
				break;
			case 'h' :
				usage(argv[0]);
				return 0;
			case 'v' :
				printf("Version %s\n", signalpp::getVersion());
				return 0;
			default:
				usage(argv[0]);
				return 1;
		}
	}

	/* Initialize environment */
	signalpp::Env env(&storage);

	/* Register client if this first run */
	if (!signalpp::Registration::everDone(storage)) {
		register_client();
	}

	/* Initialize the client */
	init(firstRun);

	return 0;
}
