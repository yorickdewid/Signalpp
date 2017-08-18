#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <memory>
#include <iostream>

namespace signalpp {

class StorageContainer {
  public:

    /* Interfaces */
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual void put(const std::string& key, int value) = 0;
    virtual bool get(const std::string& key, std::string& value) = 0;
    virtual bool get(const std::string& key, int& value) = 0;
    virtual void purge(const std::string& key) = 0;
	virtual void commit() = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

};

template<class T>
class Storage : public StorageContainer {
	std::unique_ptr<StorageContainer> m_db = nullptr;

  public:
	Storage(const std::string& name)
		: m_db{ new T(name) } {
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

	inline void commit() {
		m_db->commit();
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
