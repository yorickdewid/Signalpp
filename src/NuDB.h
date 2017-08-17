#ifndef _NUDB_H_
#define _NUDB_H_

#include "Storage.h"
#include <nudb/nudb.hpp>
#include <nudb/error.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>
#include <stdio.h>

namespace signalpp {
	class NuDB : public StorageContainer {
		nudb::store* m_db;
		bool m_db_closed = false;
	public:
		NuDB(const std::string& name) {
			initialize(name);
		}

		void initialize(const std::string& name) {
			auto const dat_path = name + std::string(".dat");
			auto const key_path = name + std::string(".key");
			auto const log_path = name + std::string(".log");
			boost::system::error_code ec;

			if (!is_file_exist(dat_path.c_str())) {
				using key_type = std::string;
				nudb::create<nudb::xxhasher>(
					dat_path, key_path, log_path,
					1,
					nudb::make_salt(),
					sizeof(key_type),
					1024,
					0.5f,
					ec);
			}
			m_db = new nudb::store();
			m_db->open(dat_path, key_path, log_path, ec);
		}

		~NuDB() {
			close();
		}

		bool is_file_exist(const char *fileName)
		{
			std::ifstream infile(fileName, std::ifstream::in);
			return infile.good();
		}

		void put(const std::string& key, const std::string& value) {
			boost::system::error_code ec;

			//note sizeof returns number of bytes, incorrect here.
			m_db->insert(key.c_str(), value.c_str(), value.length(), ec);
			printf_s(key.c_str());
			//char buffer[30];
			//sprintf(buffer, "length: %i", value.length());
			//printf_s(buffer);
			std::string test;
			get(key, test);
		}

		void put(const std::string& key, int intValue) {
			boost::system::error_code ec;

			m_db->insert(key.c_str(), (void const*)&intValue, sizeof(intValue), ec);
			printf_s(ec.message().c_str());
		}

		bool get(const std::string& key, std::string& value) { //anything really
			boost::system::error_code ec;
			bool result = false;
			m_db->fetch(key.c_str(), [&](void const* buffer, std::size_t size)
			{
				std::string ret((char const*)buffer, size);
				value = ret;
				result = true;
			}, ec);

			printf_s((ec.message()).c_str());
			return result;
		}

		bool get(const std::string& key, int& intValue) {
			boost::system::error_code ec;
			bool result = false;
			m_db->fetch(key.c_str(), [&](void const* buffer, std::size_t size)
			{
				intValue = *(const int*)buffer;
				result = true;
			}, ec);

			return result;
		}

		void purge(const std::string& key) {
			close();
			int result = remove((key + std::string(".dat")).c_str());
			result += remove((key + std::string(".key")).c_str());
			result += remove((key + std::string(".log")).c_str());
			initialize(key);
		}

		void close() {
			if (!m_db_closed) {
				boost::system::error_code ec;
				m_db->close(ec);
				delete m_db;
				m_db_closed = true;
			}
		}

		void flush() {}

	};
}

#endif // _NUDB_H_

