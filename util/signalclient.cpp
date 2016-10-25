#include <iostream>
#include <sstream>
#include <string>

#include <getopt.h>

#include <signalpp/signalpp.h>

signalpp::Storage<signalpp::Ldb> storage;

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
		return "SignalClient";
	};

	if (accountManager.registerSecondDevice(provisionUrl, confirmNumber)) {
		signalpp::Registration::markDone(storage);
	}

	// textsecure:contactsync
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

	if (!signalpp::Registration::everDone(storage)) {
		register_client();
	}

	// openInbox();

	return 0;
}
