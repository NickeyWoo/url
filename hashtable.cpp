/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#include "hashtable.hpp"

template<>
u_int64_t HashFunction(u_int64_t key)
{
	return key;
}

template<>
u_int64_t HashFunction(int64_t key)
{
	return (u_int64_t)key;
}

template<>
u_int64_t HashFunction(const char* key)
{
	return HASH(key, strlen(key));
}

template<>
u_int64_t HashFunction(std::string key)
{
	return HASH(key.c_str(), key.length());
}




