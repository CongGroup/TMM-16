#include "SecIndex.h"

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

#include "config.h"

SecIndex::SecIndex()
{
}


SecIndex::~SecIndex()
{
}

void SecIndex::EncryptIndex()
{
    for (uint32_t uiL = 0; uiL < m_uiL; uiL++)
    {
        for (uint32_t uiCur = 0; uiCur < m_uiW; uiCur++)
        {
            IndexBlock *pIndexBlock = m_pIndex + (uiL * m_uiW + uiCur);
            m_Mask(pIndexBlock, pIndexBlock->uiID, uiL);
        }
    }
}

void SecIndex::m_Mask(IndexBlock *pIndexBlock, char *pMask)
{
    pIndexBlock->uiID ^= *(uint32_t*)pMask;
    pIndexBlock->usFlag ^= *(uint16_t*)(pMask + sizeof(uint32_t));
}

void SecIndex::m_Mask(IndexBlock *pIndexBlock, uint32_t uiID, uint32_t uiL)
{
    if (pIndexBlock->usFlag == 0 && pIndexBlock->uiID == 0)
    {
        //Encrypt Empty Indexblock Mask random
        uint64_t ulEmptyMask;
        *(uint32_t*)&ulEmptyMask = rand();
        *((uint32_t*)&ulEmptyMask + 1) = rand();
        m_Mask(pIndexBlock, (char*)&ulEmptyMask);
    }
    else
    {
        uint32_t *arLsh = m_GetLsh(uiID);
        uint32_t uiLsh = arLsh[uiL];
        //Generate Mask
        char szMask[SHA256_DIGEST_LENGTH];
        PRF::Sha256(m_MaskKey, sizeof(m_MaskKey), (char*)&uiLsh, sizeof(uint32_t), szMask, SHA256_DIGEST_LENGTH);
        m_Mask(pIndexBlock, szMask);
    }
}

//return the memory size
uint64_t SecIndex::Init(key_t keyShm, uint32_t uiL, uint32_t uiAllNum, uint32_t uiB, bool bZero)
{
    //Init params
    m_uiL = uiL;
    m_uiW = uiAllNum / uiL + 1;
    m_uiProb = uiB;
    m_uiAllNum = m_uiW * m_uiL;
    m_sizeAllMemory = m_uiAllNum * sizeof(IndexBlock);
    //Init the collect kickout num
    m_uiKickoutNum = 0;
    m_uiKickBack = 0;
    //Init shm
    if (!ShmCtl::GetShm((void**)&m_pIndex, keyShm, m_sizeAllMemory))
    {
        cout << "Error on GetShm" << endl;
        return 0;
    }

    //reset memory
    if (bZero)
    {
        memset(m_pIndex, 0, m_sizeAllMemory);
    }

    //Init SemCtl lock
    // L for Counter bucket hash
    // L for lock a line for modify block
    m_semCtl.Init(keyShm + 1, uiL * 2);
    for (uint32_t uiCur = 0; uiCur < uiL * 2; uiCur++)
    {
        m_semCtl.SetSem(uiCur, 1);
    }
    //Init Bucket hash for saving counter
    m_arbukCount = new BukHash<uint32_t, uint32_t>[uiL];
    for (uint32_t uiCur = 0; uiCur < uiL; uiCur++)
    {
        m_arbukCount[uiCur].Create(keyShm + 2 + uiCur, DEF_BUKHASH_W, DEF_BUKHASH_L, bZero);
    }

    return m_sizeAllMemory;

}

//Set the master key
void SecIndex::SetKey(string strMasterKey)
{
    string sKey = strMasterKey + "Trapdoor";
    Digest::Sha256(sKey, m_TrapdoorKey, sizeof(m_TrapdoorKey));
    sKey = strMasterKey + "Mask";
    Digest::Sha256(sKey, m_MaskKey, sizeof(m_MaskKey));
}

void SecIndex::Query(uint32_t uiQueryID, uint32_t *arRet, uint32_t &uiResNum)
{
    uint32_t *arLsh = m_GetLsh(uiQueryID);
    uiResNum = 0;
    for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
    {
        uint32_t uiLsh = arLsh[uiCur];
        //Generate Trapdoor
        char szTrapdoor[SHA256_DIGEST_LENGTH];
        PRF::Sha256(m_TrapdoorKey, sizeof(m_TrapdoorKey), (char*)&uiLsh, sizeof(uint32_t), szTrapdoor, SHA256_DIGEST_LENGTH);

        for (uint32_t uiB = 0; uiB < m_uiProb; uiB++)
        {
            uint32_t uiKey = m_CombineProb(uiB, szTrapdoor);

            //Safe get the copy
            IndexBlock tempIndexBlock;
            m_SafeGet(uiKey, uiCur, &tempIndexBlock);
            m_Mask(&tempIndexBlock, uiQueryID, uiCur);
            if (tempIndexBlock.usFlag == DEF_BLOCK_FLAG)
            {
                //cout << "uiKey = " << uiKey << "   uiB = " << uiB << "  uiCur = " << uiCur << "  Find ID = " << tempIndexBlock.uiID << endl;
                *arRet++ = tempIndexBlock.uiID;
                uiResNum++;
            }
        }

    }

}


emInsertRet SecIndex::Insert_Nocache(uint32_t uiID, uint32_t uiL, uint32_t uiVal, uint32_t uiKickout)
{

    if (uiKickout >= m_uiMaxKickout)
    {
        return MaxKickout;
    }

    uint32_t *arLsh = m_GetLsh(uiID);
    char szTrapdoor[SHA256_DIGEST_LENGTH];

    for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
    {
        uint32_t uiCurL = (uiL + uiCur) % m_uiL;

        uint32_t uiLsh = arLsh[uiL];

        //Generate Trapdoor
        PRF::Sha256(m_TrapdoorKey, sizeof(m_TrapdoorKey), (char*)&uiLsh, sizeof(uint32_t), szTrapdoor, SHA256_DIGEST_LENGTH);

        for (uint32_t uiProb = 0; uiProb < m_uiProb; uiProb++)
        {

            uint32_t uiKey = m_CombineProb(uiProb, szTrapdoor);

            //Safe get the copy
            IndexBlock tempIndexBlock;
            m_SafeGet(uiKey, uiCurL, &tempIndexBlock);
            if (tempIndexBlock.uiID == 0)
            {
                //Empty
                IndexBlock *pIndexBlock = m_LockBlock(uiKey, uiCurL);
                if (pIndexBlock->uiID == 0)
                {
                    pIndexBlock->uiID = uiVal;
                    pIndexBlock->usFlag = DEF_BLOCK_FLAG;
                    m_ReleaseBlock(pIndexBlock, uiCurL);
                    return Success;
                }
                m_ReleaseBlock(pIndexBlock, uiCurL);
            }
        }
    }

    //No place to insert so kickout begin
    bool *arBool = new bool[m_uiL];
    memset(arBool, 0, m_uiL * sizeof(bool));
    arBool[uiL] = true;
    uint32_t uiAvailable;

    do
    {

        uint32_t uiVictimL = rand() % m_uiL;
        if (arBool[uiVictimL] == true)
        {
            continue;
        }

        uint32_t uiLsh = arLsh[uiVictimL];
        PRF::Sha256(m_TrapdoorKey, sizeof(m_TrapdoorKey), (char*)&uiLsh, sizeof(uint32_t), szTrapdoor, SHA256_DIGEST_LENGTH);
        uint32_t uiBegProb = rand() % m_uiProb;

        //Check the block if can be kickout
        for (uint32_t uiCur = 0; uiCur < m_uiProb; uiCur++)
        {
            uint32_t uiB = (uiBegProb + uiCur) % m_uiProb;
            uint32_t uiKey = m_CombineProb(uiB, szTrapdoor);
            //Safe get the copy
            IndexBlock tempIndexBlock;
            m_SafeGet(uiKey, uiVictimL, &tempIndexBlock);

            if (tempIndexBlock.bKickoutPath == true)
            {
                arBool[uiVictimL] = true;
                continue;
            }

            //Not on the kickout path

            IndexBlock *pIndexBlock = m_LockBlock(uiKey, uiVictimL);

            if (pIndexBlock->bKickoutPath == false)
            {
                //Begin to kickout
                pIndexBlock->bKickoutPath = true;
                uint32_t uiVictimID = pIndexBlock->uiID;
                m_ReleaseBlock(pIndexBlock, uiVictimL);

                m_uiKickoutNum++;
                emInsertRet res = Insert_Nocache(uiVictimID, uiVictimL, uiVictimID, uiKickout + 1);

                if (res == emInsertRet::Success)
                {
                    m_LockBlock(pIndexBlock, uiVictimL);

                    pIndexBlock->uiID = uiVictimID;
                    pIndexBlock->usFlag = DEF_BLOCK_FLAG;
                    pIndexBlock->bKickoutPath = false;

                    m_ReleaseBlock(pIndexBlock, uiVictimL);

                    //release the memory
                    delete[] arBool;

                    return emInsertRet::Success;
                }
                else
                {
                    m_LockBlock(pIndexBlock, uiVictimL);
                    pIndexBlock->bKickoutPath = false;
                    m_ReleaseBlock(pIndexBlock, uiVictimL);
                }

            }
            else
            {
                m_ReleaseBlock(pIndexBlock, uiVictimL);
            }

        }

        //All prob is on kickout path
        arBool[uiVictimL] = true;

        //Check the end condition
        uiAvailable = 0;
        for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
        {
            if (arBool[uiCur])
            {
                uiAvailable++;
            }
        }

    } while (uiAvailable != m_uiL);

    delete[] arBool;

    m_uiKickBack++;

    return emInsertRet::PathOccupy;

}



//Insert a lsh-id pair
emInsertRet SecIndex::Insert(uint32_t uiID, uint32_t uiL, uint32_t uiVal, uint32_t uiKickout)
{
	//Remove if don't want to count max kickout
	m_TryMaxKickout(uiKickout);
    
	
	if (uiKickout >= m_uiMaxKickout)
    {
        return MaxKickout;
    }

    uint32_t *arLsh = m_GetLsh(uiID);
    char szTrapdoor[SHA256_DIGEST_LENGTH];

    for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
    {
        uint32_t uiCurL = (uiL + uiCur) % m_uiL;

        uint32_t uiLsh = arLsh[uiCurL];

        //Generate Trapdoor
        PRF::Sha256(m_TrapdoorKey, sizeof(m_TrapdoorKey), (char*)&uiLsh, sizeof(uint32_t), szTrapdoor, SHA256_DIGEST_LENGTH);

        while (true)
        {
            uint32_t uiProb = m_GetIncCount(uiCurL, szTrapdoor);

            if (uiProb == m_uiProb)
            {
                break;
            }

            uint32_t uiKey = m_CombineProb(uiProb, szTrapdoor);

            //Safe get the copy
            IndexBlock tempIndexBlock;
            m_SafeGet(uiKey, uiCurL, &tempIndexBlock);
            if (tempIndexBlock.usFlag == 0)
            {
                //Empty
                IndexBlock *pIndexBlock = m_LockBlock(uiKey, uiCurL);
                if (pIndexBlock->usFlag == 0)
                {
                    pIndexBlock->uiID = uiVal;
                    pIndexBlock->usFlag = DEF_BLOCK_FLAG;
                    m_ReleaseBlock(pIndexBlock, uiCurL);
                    return Success;
                }
                m_ReleaseBlock(pIndexBlock, uiCurL);
            }
        }

    }

    //No place to insert so kickout begin
    bool *arBool = new bool[m_uiL];
    for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
    {
        arBool[uiCur] = false;
    }
    arBool[uiL] = true;
    uint32_t uiAvailable = 0;

    do
    {

        uint32_t uiVictimL = rand() % m_uiL;
        if (arBool[uiVictimL] == true)
        {
            continue;
        }

        uint32_t uiLsh = arLsh[uiVictimL];
        PRF::Sha256(m_TrapdoorKey, sizeof(m_TrapdoorKey), (char*)&uiLsh, sizeof(uint32_t), szTrapdoor, SHA256_DIGEST_LENGTH);
        uint32_t uiBegProb = rand() % m_uiProb;

        //Check the block if can be kickout
        for (uint32_t uiCur = 0; uiCur < m_uiProb; uiCur++)
        {
            uint32_t uiB = (uiBegProb + uiCur) % m_uiProb;
            uint32_t uiKey = m_CombineProb(uiB, szTrapdoor);
            //Safe get the copy
            IndexBlock tempIndexBlock;
            m_SafeGet(uiKey, uiVictimL, &tempIndexBlock);

            if (tempIndexBlock.bKickoutPath == true)
            {
                continue;
            }

            //Not on the kickout path

            IndexBlock *pIndexBlock = m_LockBlock(uiKey, uiVictimL);

            if (pIndexBlock->bKickoutPath == false)
            {
                //Begin to kickout
                pIndexBlock->bKickoutPath = true;
                uint32_t uiVictimID = pIndexBlock->uiID;
                if (pIndexBlock->usFlag != DEF_BLOCK_FLAG)
                {
                    cout << "ERROR : pIndexBlock->usFlag == DEF_BLOCK_FLAG" << endl;
                    break;
                }

                m_ReleaseBlock(pIndexBlock, uiVictimL);

                m_uiKickoutNum++;
                emInsertRet res = Insert(uiVictimID, uiVictimL, uiVictimID, uiKickout + 1);

                if (res == emInsertRet::Success)
                {
                    m_LockBlock(pIndexBlock, uiVictimL);

                    pIndexBlock->uiID = uiVal;
                    pIndexBlock->bKickoutPath = false;

                    m_ReleaseBlock(pIndexBlock, uiVictimL);

                    //release the memory
                    delete[] arBool;

                    return emInsertRet::Success;
                }
                else
                {
                    m_LockBlock(pIndexBlock, uiVictimL);
                    pIndexBlock->bKickoutPath = false;
                    m_ReleaseBlock(pIndexBlock, uiVictimL);
                }

            }
            else
            {
                m_ReleaseBlock(pIndexBlock, uiVictimL);
            }

        }

        //All prob is on kickout path
        arBool[uiVictimL] = true;

        //Check the end condition
        uiAvailable = 0;
        for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
        {
            if (arBool[uiCur])
            {
                uiAvailable++;
            }
        }

    } while (uiAvailable != m_uiL);

    delete[] arBool;

    m_uiKickBack++;

    return emInsertRet::PathOccupy;

}

void SecIndex::m_SafeGet(uint32_t uiKey, uint32_t uiL, IndexBlock *pOut)
{
    IndexBlock *pIndexBlock = m_Get(uiKey, uiL);
    uint8_t cv;
    do
    {
        cv = pIndexBlock->ubCV;
        memcpy(pOut, pIndexBlock, sizeof(IndexBlock));
    } while (cv & 1 || cv != pOut->ubCV);

}

uint32_t SecIndex::m_GetIncCount(uint32_t uiL, char *pTrap)
{
    uint32_t uiCountKey = m_BinaryToUint32(pTrap, SHA256_DIGEST_LENGTH);
    m_LockBukHash(uiL);
    uint32_t uiRet;
    uint32_t *puiCount = m_arbukCount[uiL].Get(uiCountKey, true);
    if (*puiCount < m_uiProb)
    {
        uiRet = (*puiCount)++;
    }
    else
    {
        uiRet = m_uiProb;
    }
    m_ReleaseBukHash(uiL);
    //cout << "uiCountKey = " << uiCountKey << " = " << uiRet << endl;
    return uiRet;
}

uint64_t SecIndex::m_BinaryToUint64(char *pData, uint32_t uiLen)
{
    uint64_t *pCur = (uint64_t*)pData;
    uint64_t uiRet = *pCur++;
    uiLen -= sizeof(uint64_t);
    while (uiLen >= sizeof(uint64_t))
    {
        uiRet ^= *pCur++;
        uiLen -= sizeof(uint64_t);
    }
    return uiRet;
}


uint32_t SecIndex::m_BinaryToUint32(char *pData, uint32_t uiLen)
{
    uint32_t *pCur = (uint32_t*)pData;
    uint32_t uiRet = *pCur++;
    uiLen -= sizeof(uint32_t);
    while (uiLen >= sizeof(uint32_t))
    {
        uiRet ^= *pCur++;
        uiLen -= sizeof(uint32_t);
    }
    return uiRet;
}

