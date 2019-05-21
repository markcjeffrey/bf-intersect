#ifndef __ABSTRACT_HASH_H__
#define __ABSTRACT_HASH_H__

#include <stdint.h>
#include <stdlib.h> // For size_t

typedef uint32_t HT;
typedef uint32_t AddrT;

template<class T, int N>
class HashAbstract
{
public:
    HashAbstract() {}
    virtual ~HashAbstract() {}

    virtual T operator()(const void* data,
                            size_t size,
                            size_t offset = 0) const = 0;
};

#endif
