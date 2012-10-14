/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#ifndef __HASHTABLE_HPP__
#define __HASHTABLE_HPP__

#include <string.h>
#include "misc.h"

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)			\
		if(p)						\
		{							\
			delete p;				\
			p = NULL;				\
		}
#endif

#ifndef SAFE_DELETE_BUFFER
	#define SAFE_DELETE_BUFFER(p)		\
		if(p)							\
		{								\
			delete [] p;				\
			p = NULL;					\
		}
#endif


template<typename KeyT>
u_int64_t HashFunction(KeyT key);

template<typename ValueT>
struct HashNode
{
	u_int64_t	ddwKey;
	ValueT		Value;
};

template<typename KeyT, typename ValueT>
class HashTable
{
public:
	static HashTable<KeyT, ValueT> CreateHashTable(size_t seed, size_t len)
	{
		HashTable<KeyT, ValueT> ht;
		ht.m_pSeedBuffer = new u_int32_t[len];
		memset(ht.m_pSeedBuffer, 0, sizeof(u_int32_t)*len);
		ht.m_dwSeedBufferSize = GetPrimeNumberArray(seed, ht.m_pSeedBuffer, len);

		for(u_int32_t i=0; i<ht.m_dwSeedBufferSize; ++i)
		{
			ht.m_dwHTBufferSize += ht.m_pSeedBuffer[i];
		}

		ht.m_pHTBuffer = new HashNode<ValueT>[ht.m_dwHTBufferSize];
		memset(ht.m_pHTBuffer, 0, ht.m_dwHTBufferSize*sizeof(HashNode<ValueT>));
		return ht;
	}

	HashTable() :
		m_dwSeedBufferSize(0),
		m_dwHTBufferSize(0),
		m_pHTBuffer(NULL),
		m_pSeedBuffer(NULL)
	{
	}

	void Delete()
	{
		SAFE_DELETE_BUFFER(m_pSeedBuffer);
		SAFE_DELETE_BUFFER(m_pHTBuffer);
	}

	ValueT* Hash(KeyT key)
	{
		u_int64_t ddwKey = HashFunction<KeyT>(key);

		u_int32_t dwOffset = 0;
		for(u_int32_t i=0; i<m_dwSeedBufferSize; ++i)
		{
			u_int32_t dwPos = (ddwKey % m_pSeedBuffer[i] + dwOffset);
			HashNode<ValueT>* pValue = &m_pHTBuffer[dwPos];
			if(pValue->ddwKey == 0 || pValue->ddwKey == ddwKey)
				return &pValue->Value;
			dwOffset += m_pSeedBuffer[i];
		}
		return NULL;
	}

private:
	u_int32_t			m_dwSeedBufferSize;
	u_int32_t			m_dwHTBufferSize;

	HashNode<ValueT>*	m_pHTBuffer;
	u_int32_t*			m_pSeedBuffer;
};

#endif // define __HASHTABLE_HPP__
