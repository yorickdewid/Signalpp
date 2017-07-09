#ifndef _MEMDB_H_
#define _MEMDB_H_

#define USE_MEMDB 1

#ifdef USE_MEMDB

#include "Storage.h"

#include <map>

namespace signalpp {

	class MemDB : public StorageContainer {
		std::map<std::string, std::string> m_str;
		std::map<std::string, int> m_int;

	public:
		MemDB(const std::string& name) {}

		void put(const std::string& key, const std::string& value) {
			m_str.insert(std::make_pair(key, value));
		}

		void put(const std::string& key, int intValue) {
			m_int.insert(std::make_pair(key, intValue));
		}

		bool get(const std::string& key, std::string& value) {
			auto search = m_str.find(key);
			if (search != m_str.end()) {
				value = search->second;
				return true;
			}
			
			return false;
		}

		bool get(const std::string& key, int& intValue) {
			auto search = m_int.find(key);
			if (search != m_int.end()) {
				intValue = search->second;
				return true;
			}

			return false;
		}

		void purge(const std::string& key) { }

		void close() {}

		void flush() {}

	};


} // namespace signalpp

#endif // _USE_MEMDB_

#endif // _MEMDB_H_
