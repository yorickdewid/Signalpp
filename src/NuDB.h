#ifndef _NUDB_H_
#define _NUDB_H_

#include "Storage.h"
#include <nudb/nudb.hpp>
#include <nudb/error.hpp>
#include <boost/system/error_code.hpp>
#include <fstream>

namespace signalpp {
	class NuDB : public StorageContainer {
		nudb::store* m_db;
	public:
		NuDB(const std::string& name) {
			auto const dat_path = name + std::string(".dat");
			auto const key_path = name + std::string(".key");
			auto const log_path = name + std::string(".log");
			boost::system::error_code ec;

			if (!is_file_exist(dat_path.c_str())) {
				using key_type = std::uint32_t;
				nudb::create<nudb::xxhasher>(
					dat_path, key_path, log_path,
					1,
					nudb::make_salt(),
					sizeof(key_type),
					nudb::block_size("."),
					0.5f,
					ec);
			}
			m_db = new nudb::store(); //important
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

			m_db->insert(key.c_str(), value.c_str(), sizeof(value), ec);
			printf_s(ec.message().c_str());
		}

		void put(const std::string& key, int intValue) {
			boost::system::error_code ec;

			m_db->insert(key.c_str(), (void const*)&intValue, sizeof(intValue), ec);
			printf_s(ec.message().c_str());
		}

		bool get(const std::string& key, std::string& value) { //anything really
			boost::system::error_code ec;

			m_db->fetch(key.c_str(), [&](void const* buffer, std::size_t size)
			{
				std::string ret((char const*)buffer, size);
				value = ret;
			}, ec);

			printf_s(ec.message().c_str());
			return true;
		}

		bool get(const std::string& key, int& intValue) {
			boost::system::error_code ec;
			m_db->fetch(key.c_str(), [&](void const* buffer, std::size_t size)
			{
				intValue = *(const int*)buffer;
			}, ec);

			return true;
		}

		void purge(const std::string& key) {
			//m_db->Delete(leveldb::WriteOptions(), key);
		}

		void close() {
			boost::system::error_code ec;
			m_db->close(ec);
			delete m_db;
		}

		void flush() {}

	};
}

#endif // _NUDB_H_

