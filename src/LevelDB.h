#ifndef _LEVELDB_H_
#define _LEVELDB_H_

#ifdef USE_LEVELDB

#include "Storage.h"

#include <memory>
#include <iostream>

#include <leveldb/db.h>

#define DEFAULT_NAME "indexdb"

namespace signalpp {

	class LevelDB : public StorageContainer {
		leveldb::DB* m_db;
		leveldb::Options m_options;

	public:
		LevelDB(const std::string& name) {
			m_options.create_if_missing = true;

			leveldb::Status status = leveldb::DB::Open(m_options, name, &m_db);

			if (!status.ok()) {
				std::cerr << "Unable to open/create database " << name << std::endl;
				std::cerr << status.ToString() << std::endl;
			}
		}

		~LevelDB() {
			close();
		}

		void put(const std::string& key, const std::string& value) {
			m_db->Put(leveldb::WriteOptions(), key, value);
		}

		void put(const std::string& key, int intValue) {
			leveldb::Slice value((char *)&intValue, sizeof(int));

			m_db->Put(leveldb::WriteOptions(), key, value);
		}

		bool get(const std::string& key, std::string& value) {
			leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, &value);
			return status.ok();
		}

		bool get(const std::string& key, int& intValue) {
			std::string value;

			leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, &value);

			intValue = (uint8_t)value[3] << 24 | (uint8_t)value[2] << 16 | (uint8_t)value[1] << 8 | (uint8_t)value[0];

			return status.ok();
		}

		void purge(const std::string& key) {
			m_db->Delete(leveldb::WriteOptions(), key);
		}

		void close() {
			delete m_db;
		}

		void flush() {}

	};


} // namespace signalpp

#endif // _USE_LEVELDB_

#endif // _LEVELDB_H_
