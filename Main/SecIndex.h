#ifndef __SEC_INDEX_H__
#define __SEC_INDEX_H__

#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>
#include <string>
#include <stdint.h>
#include <map>
#include <vector>

#include "../Caravel/Digest.h"
#include "../Caravel/PRF.h"
#include "../Caravel/SemCtl.h"
#include "../Caravel/ShmCtl.h"
#include "../Caravel/TimeDiff.h"
#include "../Caravel/BukHash.h"

using namespace std;
using namespace caravel;

#define DEF_BLOCK_FLAG 12345

typedef struct stIndexBlock
{
	uint32_t uiID;
	uint16_t usFlag;
	uint8_t ubCV;
	bool bKickoutPath;
}IndexBlock;

enum emInsertRet
{
	Success = 0,
	MaxKickout,
	PathOccupy
};

class SecIndex
{
public:
	SecIndex();
	~SecIndex();

	//return the memory size
	uint64_t Init(key_t keyShm, uint32_t uiL, uint32_t uiAllNum, uint32_t uiB, bool bZero);

	//Set the master key
	void SetKey(string strMasterKey);

	//Set the max kickout
	void SetMaxKickout(uint32_t uiMaxKickout)
	{
		m_uiMaxKickout = uiMaxKickout;
	}

	//Set the LSH meta data
	void SetLshMetaData(uint32_t *arLsh)
	{
		m_arLsh = arLsh;
	}

	uint32_t GetKickBackNum()
	{
		return m_uiKickBack;
	}



	//Insert a lsh-id pair
	emInsertRet Insert(uint32_t uiID, uint32_t uiL, uint32_t uiVal, uint32_t uiKickout = 0);

	//Insert a lsh-id pair without cache
	emInsertRet Insert_Nocache(uint32_t uiID, uint32_t uiL, uint32_t uiVal, uint32_t uiKickout = 0);

	//Query a sets of results
	void Query(uint32_t uiQueryID, uint32_t *arRet, uint32_t &uiResNum);

	void ShowBukState(uint32_t uiBukID)
	{
		m_arbukCount[uiBukID].PrintState();
	}

	void EncryptIndex();

	uint32_t GetKickoutNum()
	{
		return m_uiKickoutNum;
	}

	uint32_t GetInsertMaxKickout()
	{
		return m_uiInsertMaxKickout;
	}

private:

	void m_Mask(IndexBlock *pIndexBlock, uint32_t uiID, uint32_t uiL);
	inline void m_Mask(IndexBlock *pIndexBlock, char *pMask);

	inline uint32_t m_BinaryToUint32(char *pData, uint32_t uiLen);

	inline uint64_t m_BinaryToUint64(char *pData, uint32_t uiLen);

	//Inner function for lock

	void m_LockBukHash(uint32_t uiL)
	{
		m_semCtl.ModSem(uiL, 1);
	}

	void m_ReleaseBukHash(uint32_t uiL)
	{
		m_semCtl.ModSem(uiL, -1);
	}

	void m_LockIndex(uint32_t uiL)
	{
		m_semCtl.ModSem(m_uiL + uiL, 1);
	}

	void m_ReleaseIndex(uint32_t uiL)
	{
		m_semCtl.ModSem(m_uiL + uiL, -1);
	}

	IndexBlock *m_LockBlock(uint32_t uiKey, uint32_t uiL)
	{
		IndexBlock *pIndexBlock = m_Get(uiKey, uiL);
		m_LockIndex(uiL);
		pIndexBlock->ubCV++;
		return pIndexBlock;
	}

	void m_LockBlock(IndexBlock *pIndexBlock, uint32_t uiL)
	{
		m_LockIndex(uiL);
		pIndexBlock->ubCV++;
	}

	void m_ReleaseBlock(IndexBlock *pIndexBlock, uint32_t uiL)
	{
		pIndexBlock->ubCV++;
		m_ReleaseIndex(uiL);
	}

	uint32_t m_CombineProb(uint32_t uiProb, char *szTrap)
	{
		char szBuf[SHA256_DIGEST_LENGTH];
		PRF::Sha256((char*)&uiProb, sizeof(uint32_t), szTrap, SHA256_DIGEST_LENGTH, szBuf, SHA256_DIGEST_LENGTH);
		return m_BinaryToUint32(szBuf, SHA256_DIGEST_LENGTH);
	}

	IndexBlock* m_Get(uint32_t uiKey, uint32_t uiL)
	{
		return m_pIndex + (uiL * m_uiW + uiKey % m_uiW);
	}

	void m_SafeGet(uint32_t uiKey, uint32_t uiL, IndexBlock *pOut);

	uint32_t *m_GetLsh(uint32_t uiID)
	{
		size_t uiOff = uiID;
		uiOff *= m_uiL;
		return m_arLsh + uiOff;
	}

	//For remember the kick back times
	uint32_t m_uiKickBack;


	//For use signal lock
	SemCtl m_semCtl;
	//For cache counter
	BukHash<uint32_t, uint32_t> *m_arbukCount;
	uint32_t m_GetIncCount(uint32_t uiL, char *pTrap);

	//Lsh meta data
	uint32_t *m_arLsh;

	//Max kickout times of building index
	uint32_t m_uiMaxKickout;

	//collect the kickout times
	uint32_t m_uiKickoutNum;

	//For index information
	uint32_t m_uiAllNum;
	uint32_t m_uiW;
	uint32_t m_uiL;
	uint32_t m_uiProb;
	size_t m_sizeAllMemory;
	//index head pointer
	IndexBlock *m_pIndex;

	uint32_t m_uiInsertMaxKickout = 0;

	void m_TryMaxKickout(uint32_t k)
	{
		m_uiInsertMaxKickout = m_uiInsertMaxKickout > k ? m_uiInsertMaxKickout : k;
	}

	//Keys
	char m_TrapdoorKey[SHA256_DIGEST_LENGTH];
	char m_MaskKey[SHA256_DIGEST_LENGTH];

};




#endif
