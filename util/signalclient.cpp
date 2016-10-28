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

	// if (messageReceiver) {
	// 	messageReceiver.close();
	// }

	std::string username = storage.get("number_id");
	std::string password = storage.get("password");
	std::string signalingKey = storage.get("signaling_key");

	/* Initialize the socket and start listening for messages */
	signalpp::MessageReceiver messageReceiver(&storage,
		signalpp::serverUrl,
		signalpp::serverPorts,
		username, password, signalingKey,
		signalpp::attachmentServerUrl);

	// messageReceiver.onMmessage(onMessageReceived);
	// messageReceiver.onReceipt(onDeliveryReceipt);
	// messageReceiver.onContact(onContactReceived);
	// messageReceiver.onGroup(onGroupReceived);
	// messageReceiver.onSent(onSentMessage);
	// messageReceiver.onRead(onReadReceipt);
	// messageReceiver.onError(onError);

	// signalpp::MessageSender messageSender(&storage,
	//	signalpp::serverUrl,
	//	signalpp::serverPorts,
	//	username, password, signalingKey,
	//	signalpp::attachmentServerUrl);

	if (firstRun) {
		int device_id = 0;
		if (!storage.get("device_id", device_id))
			return;
		if (!device_id || device_id == 1)
			return;

	// 	SyncRequest syncRequest(messageSender, messageReceiver);
		
	//	syncRequest.onSuccess([] () {
	//		SIGNAL_LOG_INFO << "Sync successful";
	// 		storage.put("synced_at", Date.now());
	// 		window.dispatchEvent(new Event('textsecure:contactsync'));
	//	});

	//	syncRequest.onTimeout([] () {
	//		SIGNAL_LOG_INFO << "Sync timed out";
	// 		window.dispatchEvent(new Event('textsecure:contactsync'));
	//	});
	}
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

	/* Register client if this first run */
	if (!signalpp::Registration::everDone(storage)) {
		register_client();
	}

	/* Initialize the client */
	init(firstRun);

	return 0;
}
