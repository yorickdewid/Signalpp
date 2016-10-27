#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <memory>

#include <leveldb/db.h>

#define DEFAULT_NAME "indexdb"

namespace signalpp {

class StorageContainer {
  public:

    /* Interfaces */
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual void put(const std::string& key, int value) = 0;
    virtual bool get(const std::string& key, std::string& value) = 0;
    virtual bool get(const std::string& key, int& value) = 0;
    virtual void purge(const std::string& key) = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

};

class Ldb : public StorageContainer {
	leveldb::DB* m_db;
	leveldb::Options m_options;

  public:
	Ldb(const std::string& name) {
	    m_options.create_if_missing = true;
		
		leveldb::Status status = leveldb::DB::Open(m_options, name, &m_db);

		if (!status.ok()) {
			std::cerr << "Unable to open/create database " << name << std::endl;
			std::cerr << status.ToString() << std::endl;
		}
	}

	~Ldb() {
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

template<class T>
class Storage : public StorageContainer {
	std::unique_ptr<StorageContainer> m_db;

  public:
	Storage(const std::string& name = DEFAULT_NAME) : m_db(new T(name)) {
	}

	inline void put(const std::string& key, const std::string& value) {
		m_db->put(key, value);
	}

	inline void put(const std::string& key, int value) {
		m_db->put(key, value);
	}

	inline bool get(const std::string& key, std::string& value) {
		return m_db->get(key, value);
	}

	inline bool get(const std::string& key, int& value) {
		return m_db->get(key, value);
	}

	inline std::string get(const std::string& key) {
		std::string value;
		m_db->get(key, value);
		return value;
	}

	inline void purge(const std::string& key) {
		m_db->purge(key);
	}
	
	inline void close() {
		m_db->close();
	}

	inline void flush() {
		m_db->flush();
	}
};

} // namespace signalpp

#endif // _STORAGE_H_
