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
			std::string file_name = std::string("C:\\") + name + std::string(".udb");
			if (!is_file_exist(file_name.c_str())) {
				int rc = unqlite_open(&m_db, file_name.c_str(), UNQLITE_OPEN_CREATE);
				if (rc != UNQLITE_OK) { return; }
			}
			else {
				int rc = unqlite_open(&m_db, file_name.c_str(), UNQLITE_OPEN_READWRITE);
				if (rc != UNQLITE_OK) { return; }
			}
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
			int rc = unqlite_kv_store(m_db, key.c_str(), -1, value.c_str(), value.length());
			if (rc != UNQLITE_OK) {
				if (rc == UNQLITE_EXISTS) {
					printf_s("Record exists \n");
				}
				return;
			}
		}

		void put(const std::string& key, int intValue) {
			int rc = unqlite_kv_store(m_db, key.c_str(), -1, &intValue, sizeof(intValue));
			if (rc != UNQLITE_OK) {
				if (rc == UNQLITE_EXISTS) {
					printf_s("Record exists \n");
				}
				return;
			}
		}

		void put(const std::string& key, nlohmann::json& value) {
			put(key, value.dump()); // throw or smt..
		}

		bool get(const std::string& key, std::string& value) {
			unqlite_int64 nBytes;
			int rc = -1;
			rc = unqlite_kv_fetch(m_db, key.c_str(), -1, NULL, &nBytes);
			if (rc == UNQLITE_NOTFOUND) { // -6
				printf_s("key does not exist");
				return false;
			}
			void* buffer = malloc(nBytes);
			rc = unqlite_kv_fetch(m_db, key.c_str(), -1, buffer, &nBytes);
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
			rc = unqlite_kv_fetch(m_db, key.c_str(), -1, NULL, &nBytes);
			if (rc == UNQLITE_NOTFOUND) { // -6
				printf_s("key does not exist");
				return false;
			}
			void* buffer = malloc(nBytes);
			rc = unqlite_kv_fetch(m_db, key.c_str(), -1, buffer, &nBytes);
			intValue = *(const int*)buffer;
			if (rc != UNQLITE_OK) {
				return false;
			}
			return true;
		}

		bool get(const std::string& key, nlohmann::json& value) {
			std::string serialized_json;
			bool result = get(key, serialized_json);
			if (serialized_json.length() > 0) {
				value = nlohmann::json::parse(serialized_json);
			}
			return result;
		}

		void commit() {
			unqlite_commit(m_db);
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

