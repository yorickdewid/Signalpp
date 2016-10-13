#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <memory>

#include <leveldb/db.h>

namespace sinl {

class StorageContainer {
  public:

    /* Interfaces */
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual void get(const std::string& key, std::string& value) = 0;
    virtual void purge(const std::string& key) = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

};

class Database : public StorageContainer {
	leveldb::DB* m_db;
	leveldb::Options m_options;

  public:

	Database(const std::string& name) {
	    m_options.create_if_missing = true;
		
		leveldb::Status status = leveldb::DB::Open(m_options, name, &m_db);

		if (!status.ok()) {
			std::cerr << "Unable to open/create database " << name << std::endl;
			std::cerr << status.ToString() << std::endl;
		}
	}

	~Database() {
		close();
	}

	void put(const std::string& key, const std::string& value) {
		m_db->Put(leveldb::WriteOptions(), key, value);
	}

	void get(const std::string& key, std::string& value) {
		m_db->Get(leveldb::ReadOptions(), key, &value);
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
	Storage(const std::string& name) : m_db(new T(name)) {
	}

	inline void put(const std::string& key, const std::string& value) {
		m_db->put(key, value);
	}

	inline void get(const std::string& key, std::string& value) {
		m_db->get(key, value);
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

} // namespace sinl

#endif // _STORAGE_H_
