#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <leveldb/db.h>

namespace sinl {

class Storage {
public:

    /* Interfaces */
    virtual void put(const std::string& data) = 0;
    virtual void get(const std::string& data) = 0;
    virtual void close() = 0;

};

} // namespace sinl

#endif // _STORAGE_H_
