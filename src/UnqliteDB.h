#ifndef _UNQLITEDB_H_
#define _UNQLITEDB_H_

#include "Storage.h"
extern "C" {
#include "unqlite.h"
}
#include <fstream>
#include <stdio.h>

namespace signalpp {
	class UnqliteDB : public StorageContainer {
		unqlite *m_db;
		bool m_db_closed = false;
	public:
		UnqliteDB(const std::string& name) {
			initialize(name);
		}

		void initialize(const std::string& name) {
			int rc = unqlite_open(&m_db, (name + std::string(".udb")).c_str(), UNQLITE_OPEN_CREATE);
			if (rc != UNQLITE_OK) { return; }
		}

		~UnqliteDB() {
			close();
		}

		bool is_file_exist(const char *fileName)
		{
			std::ifstream infile(fileName, std::ifstream::in);
			return infile.good();
		}

		void put(const std::string& key, const std::string& value) {
			int rc = unqlite_kv_store(m_db, key.c_str(), sizeof(key), value.c_str(), value.length());
			if (rc != UNQLITE_OK) {
				//Insertion fail, Hande error (See below)
				return;
			}
		}

		void put(const std::string& key, int intValue) {
			int rc = unqlite_kv_store(m_db, key.c_str(), sizeof(key), &intValue, sizeof(intValue));
			printf_s("hold");
			if (rc != UNQLITE_OK) {
				//Insertion fail, Hande error (See below)
				return;
			}
		}

		bool get(const std::string& key, std::string& value) {
			unqlite_int64 nBytes; // -1 gives different behavior
			int rc = -1;
			rc = unqlite_kv_fetch(m_db, key.c_str(), -1, NULL, &nBytes);
			void* buffer = malloc(nBytes);
			rc = unqlite_kv_fetch(m_db, key.c_str(), sizeof(key), buffer, &nBytes);
			std::string ret((char const*)buffer, nBytes);
			value = ret;
			if (rc != UNQLITE_OK) {
				return false;
			}
			return true;
		}

		bool get(const std::string& key, int& intValue) {
			unqlite_int64 nBytes;
			int rc = -1;
			rc = unqlite_kv_fetch(m_db, key.c_str(), sizeof(key), NULL, &nBytes); // passing -1 as keylength in this case gives key not found behavior
			if (rc == UNQLITE_NOTFOUND) { // -6
				printf_s("key does not exist");
				return false;
			}
			void* buffer = malloc(nBytes);
			rc = unqlite_kv_fetch(m_db, key.c_str(), sizeof(key), buffer, &nBytes);
			intValue = *(const int*)buffer;
			if (rc != UNQLITE_OK) {
				return false;
			}
			return true;
		}

		void purge(const std::string& key) {
			unqlite_kv_delete(m_db,key.c_str(), key.length());
		}

		void close() {
			unqlite_close(m_db);
		}

		void flush() {}

	};
}

#endif // _UNQLITEDB_H_

