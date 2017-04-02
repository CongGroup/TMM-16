#ifndef __SSECUCKOO_H__
#define __SSECUCKOO_H__

#include "SSECuckoo.h"

#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <stdio.h>


#include "PRF.h"
#include "Digest.h"

using namespace std;

namespace caravel {


	SSECuckoo::SSECuckoo()
	{

	}

	SSECuckoo::~SSECuckoo()
	{

	}


	void SSECuckoo::SetKey(string sMasterKey)
	{
		string sKey = sMasterKey + "Key1";
		Digest::Sha256(sKey, m_Key1, sizeof(m_Key1));
		//Digest::Sha256(sMasterKey + "Key2", m_Key2, sizeof(m_Key2));
		//Digest::Sha256(sMasterKey + "Key3", m_Key3, sizeof(m_Key3));

	}

	uint64_t SSECuckoo::Size()
	{
		return m_ulMemSize;
	}


	uint64_t SSECuckoo::Size(uint32_t uiNum, double dLoad, uint32_t uiB, uint32_t uiS)
	{
		uint32_t uiW = (uiNum / dLoad) / (2 * uiB) + 1;
		return (uiS + sizeof(m_uiFlag)) * 2 * uiB * uiW;
	}

	uint64_t SSECuckoo::Init(char *p, uint32_t uiNum, double dLoad, uint32_t uiB, uint32_t uiS, uint32_t uiMaxKick, bool bReset)
	{
		uint32_t uiW = (uiNum / dLoad) / (2 * uiB) + 1;
		return Init(p, uiW, uiB, uiS, uiMaxKick, bReset);
	}

	uint64_t SSECuckoo::Init(char *p, uint32_t uiW, uint32_t uiB, uint32_t uiS, uint32_t uiMaxKick, bool bReset)
	{

		//Init the Params
		m_pAll = p;
		m_uiW = uiW;
		m_uiB = uiB;
		m_uiS = uiS;
		m_uiMaxKick = uiMaxKick;
		m_uiCurKick = 0;

		//Compute the Line Pointer
		m_uiLineCnt = uiB * uiW;
		m_uiAllCnt = m_uiLineCnt * 2;

		//Size of Flag and uiSize
		m_uiSizePerEntry = sizeof(m_uiFlag) + uiS;
		m_uiSizePerBucket = m_uiSizePerEntry * uiB;

		//Compute the offset and set the pointer of each line
		m_Line[0] = m_pAll;
		m_Line[1] = m_pAll + m_uiLineCnt * m_uiSizePerEntry;

		//Init the temp memory for caching the Xor.
		m_arTempXor = new uint32_t[m_uiAllCnt];
		memset(m_arTempXor, 0, sizeof(m_arTempXor));

		m_ulMemSize = m_uiAllCnt * m_uiSizePerEntry;
		//memset all the memory
		if (bReset)
		{
			memset(m_pAll, 0, m_ulMemSize);
		}

		//Init the temp memory for caching the T.
		m_arMask = new char[SHA256_DIGEST_LENGTH * m_uiAllCnt];
		memset(m_arMask, 0, sizeof(m_arMask));

		//Init the temp memory for decryption
		m_TempGet = new char[m_uiSizePerEntry];

		//Set the Flag
		m_uiFlag = 12345678;

		return m_ulMemSize;

	}

	bool SSECuckoo::Get(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parRet)
	{
		uint32_t uiFind = 0;

		char szF[SHA256_DIGEST_LENGTH];
		//f <- fingerprint (V)
		Digest::Sha256(pKey, uiKey, szF, sizeof(szF));
		//t <- PRF<f, k1>
		char szT[SHA256_DIGEST_LENGTH];
		PRF::Sha256(m_Key1, sizeof(m_Key1), szF, sizeof(szF), szT, sizeof(szT));

		//for j = 1 to uiC
		for (uint32_t uiC = 0; uiC < uiCnt; uiC++)
		{

			//i1 <- PRF<t, j>
			char szI1[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)&uiC, sizeof(uiC), szT, sizeof(szT), szI1, sizeof(szI1));
			//For encryption Mask = H < t, j >
			char szMask[SHA256_DIGEST_LENGTH];
			uint32_t uiMaskC = uiC + 10000;
			PRF::Sha256((char*)&uiMaskC, sizeof(uiMaskC), szT, sizeof(szT), szMask, sizeof(szMask));
			//compute the pos Idx1
			//0 means Idx1   1 means Idx2
			uint32_t uiIdx[2];
			uiIdx[0] = *(uint32_t*)szI1 % m_uiW;
			char *pBucket = m_pAll + m_uiSizePerBucket * uiIdx[0];
			char *pGet = _GetEntrys(pBucket, szMask);
			if (NULL != pGet)
			{
				//Find it
				memcpy(parRet[uiC], pGet, m_uiS);
				uiFind++;
				continue;
			}
			//Not in. Change another.
			//compute the XOR
			char szI2[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)uiIdx, sizeof(uint32_t), szT, sizeof(szT), szI2, sizeof(szI2));
			uint32_t uiXor = *(uint32_t*)szI2 % m_uiW;
			//compute the Idx 2
			uiIdx[1] = (uiXor ^ uiIdx[0]) % m_uiW;
			pBucket = m_pAll + m_uiSizePerBucket * (m_uiW + uiIdx[1]);
			pGet = _GetEntrys(pBucket, szMask);
			if (NULL != pGet)
			{
				//Find it
				parRet[uiC] = pGet;
				uiFind++;
				continue;
			}
		}
		/*

		if(uiFind != uiCnt)
		{
			cout<<"uiFind"<<uiFind<<endl;
		}

		*/

		return uiFind == uiCnt;

	}

	uint32_t SSECuckoo::GetMapNum()
	{
		return m_uiUseMap;
	}

	uint32_t SSECuckoo::Insert(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parVal, uint32_t uiVal)
	{
		char szF[SHA256_DIGEST_LENGTH];
		//f <- fingerprint (V)
		Digest::Sha256(pKey, uiKey, szF, sizeof(szF));
		//t <- PRF<f, k1>
		char szT[SHA256_DIGEST_LENGTH];
		PRF::Sha256(m_Key1, sizeof(m_Key1), szF, sizeof(szF), szT, sizeof(szT));

		uint32_t uiInsertNum = 0;

		//for j = 1 to uiC
		for (uint32_t uiC = 0; uiC < uiCnt; uiC++)
		{
			bool bSuccess = false;

			//i1 <- PRF<t, j>
			char szI1[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)&uiC, sizeof(uiC), szT, sizeof(szT), szI1, sizeof(szI1));
			//For encryption Mask = H < t, j >
			char szMask[SHA256_DIGEST_LENGTH];
			uint32_t uiMaskC = uiC + 10000;
			PRF::Sha256((char*)&uiMaskC, sizeof(uiMaskC), szT, sizeof(szT), szMask, sizeof(szMask));
			//compute the pos Idx1
			//0 means Idx1   1 means Idx2
			uint32_t uiIdx[2];
			uiIdx[0] = *(uint32_t*)szI1 % m_uiW;

			//Check the first line
			uint32_t uiOffset = uiIdx[0];

			//compute the XOR
			char szI2[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)uiIdx, sizeof(uint32_t), szT, sizeof(szT), szI2, sizeof(szI2));
			uint32_t uiXor = *(uint32_t*)szI2;

			//Find the Offset
			for (uint32_t uiCur = 0; uiCur < m_uiB; uiCur++)
			{

				char *p = m_Line[0] + uiIdx[0] * m_uiSizePerBucket + (uiCur * m_uiSizePerEntry);

				uint32_t uiSeed = uiIdx[0] * m_uiB + uiCur;

				char szTempMask[SHA256_DIGEST_LENGTH];
				PRF::Sha256(m_Key1, sizeof(m_Key1), (char*)&uiSeed, sizeof(uiSeed), szTempMask, sizeof(szTempMask));

				uint32_t uiFlag;

				for (uint32_t uiX = 0; uiX < sizeof(uiFlag); uiX++)
				{
					((char*)&uiFlag)[uiX] = p[m_uiS + uiX] ^ szTempMask[m_uiS + uiX];
				}

				if (uiFlag == 0)
				{
					//Empty
					//Set flag
					*(uint32_t*)(m_Line[0] + uiIdx[0] * m_uiSizePerBucket + (uiCur * m_uiSizePerEntry) + m_uiS) = m_uiFlag;

					//Insert data
					for (uint32_t uiX = 0; uiX < m_uiSizePerEntry; uiX++)
					{
						p[uiX] = parVal[uiC][uiX] ^ szMask[uiX];
					}

					uiInsertNum++;

					bSuccess = true;
					break;

				}

			}

			if (bSuccess) continue;

			//First Line Full

			uiIdx[1] = (uiXor ^ uiIdx[0]) % m_uiW;

			//Check the second Line
			for (uint32_t uiCur = 0; uiCur < m_uiB; uiCur++)
			{

				char *p = m_Line[1] + uiIdx[1] * m_uiSizePerBucket + (uiCur * m_uiSizePerEntry);

				uint32_t uiSeed = m_uiLineCnt * m_uiSizePerEntry + uiIdx[1] * m_uiB + uiCur;

				char szTempMask[SHA256_DIGEST_LENGTH];
				PRF::Sha256(m_Key1, sizeof(m_Key1), (char*)&uiSeed, sizeof(uiSeed), szTempMask, sizeof(szTempMask));

				uint32_t uiFlag;

				for (uint32_t uiX = 0; uiX < sizeof(uiFlag); uiX++)
				{
					((char*)&uiFlag)[uiX] = p[m_uiS + uiX] ^ szTempMask[m_uiS + uiX];
				}

				if (uiFlag == 0)
				{
					//Empty
					//Set flag
					*(uint32_t*)(m_Line[1] + uiIdx[1] * m_uiSizePerBucket + (uiCur * m_uiSizePerEntry) + m_uiS) = m_uiFlag;

					//Insert data
					for (uint32_t uiX = 0; uiX < m_uiSizePerEntry; uiX++)
					{
						p[uiX] = parVal[uiC][uiX] ^ szMask[uiX];
					}

					uiInsertNum++;

					bSuccess = true;
					break;

				}

			}

			if (bSuccess) continue;

			//All is full use map
			m_mapInsert[*(uint32_t*)pKey] = *(uint32_t*)parVal[uiC];
			m_uiUseMap++;

		}
		
		return uiInsertNum;

	}

	int SSECuckoo::Put(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parVal, uint32_t uiVal)
	{

		char szF[SHA256_DIGEST_LENGTH];
		//f <- fingerprint (V)
		Digest::Sha256(pKey, uiKey, szF, sizeof(szF));
		//t <- PRF<f, k1>
		char szT[SHA256_DIGEST_LENGTH];
		PRF::Sha256(m_Key1, sizeof(m_Key1), szF, sizeof(szF), szT, sizeof(szT));

		//for j = 1 to uiC
		for (uint32_t uiC = 0; uiC < uiCnt; uiC++)
		{

			//i1 <- PRF<t, j>
			char szI1[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)&uiC, sizeof(uiC), szT, sizeof(szT), szI1, sizeof(szI1));
			//For encryption Mask = H < t, j >
			char szMask[SHA256_DIGEST_LENGTH];
			uint32_t uiMaskC = uiC + 10000;
			PRF::Sha256((char*)&uiMaskC, sizeof(uiMaskC), szT, sizeof(szT), szMask, sizeof(szMask));
			//compute the pos Idx1
			//0 means Idx1   1 means Idx2
			uint32_t uiIdx[2];
			uiIdx[0] = *(uint32_t*)szI1 % m_uiW;

			//compute the XOR
			char szI2[SHA256_DIGEST_LENGTH];
			PRF::Sha256((char*)uiIdx, sizeof(uint32_t), szT, sizeof(szT), szI2, sizeof(szI2));
			uint32_t uiXor = *(uint32_t*)szI2;

			//for return the offset of Entry
			uint32_t uiB;
			char *pData = _SetEntrys(m_Line[0] + uiIdx[0] * m_uiSizePerBucket, uiB);
			if (NULL != pData)
			{
				//Empty and set the data
				//the user must keep in mind that uiVal must greater than uiS !
				memcpy(pData, parVal[uiC], uiVal);
				m_arTempXor[m_uiB * uiIdx[0] + uiB] = uiXor;
				memcpy(m_arMask + SHA256_DIGEST_LENGTH * (uiIdx[0] * m_uiB + uiB), szMask, sizeof(szMask));
				continue;
			}
			//full and set the second line

			uiIdx[1] = (uiXor ^ uiIdx[0]) % m_uiW;
			//try to set the data
			pData = _SetEntrys(m_Line[1] + uiIdx[1] * m_uiSizePerBucket, uiB);
			if (NULL != pData)
			{
				//Empty and set the data
				//the user must keep in mind that uiVal must greater than uiS !
				memcpy(pData, parVal[uiC], uiVal);
				m_arTempXor[m_uiB * (m_uiW + uiIdx[1]) + uiB] = uiXor;
				memcpy(m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW + uiIdx[1]) * m_uiB + uiB), szMask, sizeof(szMask));
				continue;
			}

			//If it still full , prepare to kickout
			uint32_t uiRand = rand();
			uint32_t uiLine = uiRand % 2;
			uiB = uiRand % m_uiB;
			if (_Move(uiLine, uiIdx[uiLine], uiB, m_uiMaxKick - 1))
			{
				//Move success
				memcpy(m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry, parVal[uiC], uiVal);
				*(uint32_t*)(m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry + m_uiS) = m_uiFlag;
				m_arTempXor[m_uiB * (m_uiW * uiLine + uiIdx[uiLine]) + uiB] = uiXor;
				memcpy(m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW * uiLine + uiIdx[uiLine]) * m_uiB + uiB), szMask, sizeof(szMask));
				continue;
			}
			else
			{
				//MaxKickout
				return -1;
			}

		}
		return 0;
	}

	uint32_t SSECuckoo::GetKick()
	{
		return m_uiCurKick;
	}


	bool SSECuckoo::_Move(uint32_t uiLine, uint32_t uiI, uint32_t uiB, uint32_t uiKick)
	{
		//Check MaxKick
		if (0 == uiKick)
		{
			return false;
		}

		m_uiCurKick++;

		uint32_t uiIdx[2];
		uiIdx[uiLine] = uiI;
		//Get the XOR
		uint32_t uiXor = m_arTempXor[m_uiB * (m_uiW * uiLine + uiI) + uiB];
		//Init the Next Line
		uint32_t uiNext = (uiLine + 1) % 2;
		uiIdx[uiNext] = (uiIdx[uiLine] ^ uiXor) % m_uiW;
		uint32_t uiNewB;
		char *pData = _SetEntrys(m_Line[uiNext] + uiIdx[uiNext] * m_uiSizePerBucket, uiNewB);

		//for maybe new place
		uint32_t uiRand;
		uint32_t uiKickLine;
		uint32_t uiKickB;

		if (NULL != pData)
		{
			//Next Place not Full

			//Empty and set the data
			memcpy(pData, m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry, m_uiSizePerEntry);
			memset(m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry, 0, m_uiSizePerEntry);
			m_arTempXor[m_uiB * (m_uiW * uiNext + uiIdx[uiNext]) + uiNewB] = uiXor;
			memcpy(m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW * uiNext + uiIdx[uiNext]) * m_uiB + uiNewB), m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW * uiLine + uiIdx[uiLine]) * m_uiB + uiB), SHA256_DIGEST_LENGTH);
			//Move success
			return true;

		}
		else
		{
			//if not empty use _Move to move.
			do
			{
				uiRand = rand();
				uiKickLine = uiRand % 2;
				uiKickB = uiRand % m_uiB;
			} while (uiKickLine == uiLine && uiB == uiKickB);

			if (!_Move(uiKickLine, uiIdx[uiKickLine], uiKickB, uiKick - 1))
			{
				return false;
			}

			//Move to the moved data
			memcpy(m_Line[uiKickLine] + uiIdx[uiKickLine] * m_uiSizePerBucket + uiKickB * m_uiSizePerEntry, m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry, m_uiSizePerEntry);
			memset(m_Line[uiLine] + uiIdx[uiLine] * m_uiSizePerBucket + uiB * m_uiSizePerEntry, 0, m_uiSizePerEntry);
			m_arTempXor[m_uiB * (m_uiW * uiKickLine + uiIdx[uiKickLine]) + uiKickB] = uiXor;
			memcpy(m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW * uiKickLine + uiIdx[uiKickLine]) * m_uiB + uiKickB), m_arMask + SHA256_DIGEST_LENGTH * ((m_uiW * uiLine + uiIdx[uiLine]) * m_uiB + uiB), SHA256_DIGEST_LENGTH);
		}

	}

	uint32_t SSECuckoo::GetEmpty()
	{
		uint32_t uiRet = 0;

		for (uint32_t uiCur = 0; uiCur < m_uiAllCnt; uiCur++)
		{
			char *pData = m_pAll + m_uiSizePerEntry * uiCur;

			char szMask[SHA256_DIGEST_LENGTH];
			uint32_t uiMaskC = uiCur;
			PRF::Sha256(m_Key1, sizeof(m_Key1), (char*)&uiMaskC, sizeof(uiMaskC), szMask, sizeof(szMask));
			uint32_t uiFlag;

			for (uint32_t uiX = 0; uiX < sizeof(uiFlag); uiX++)
			{
				((char*)&uiFlag)[uiX] = pData[m_uiS + uiX] ^ szMask[m_uiS + uiX];
			}

			if (uiFlag == 0)
			{
				uiRet++;
			}

		}

		return uiRet;
	}

	int SSECuckoo::Encrypt()
	{
		uint32_t uiRet = 0;

		for (uint32_t uiCur = 0; uiCur < m_uiAllCnt; uiCur++)
		{
			char *pData = m_pAll + m_uiSizePerEntry * uiCur;
			char *pXor = m_arMask + SHA256_DIGEST_LENGTH * uiCur;

			//Check Empty

			if (*(uint32_t*)(m_pAll + (uiCur * m_uiSizePerEntry) + m_uiS) == 0)
			{
				uiRet++;
				//Empty
				char szMask[SHA256_DIGEST_LENGTH];
				uint32_t uiMaskC = uiCur;
				PRF::Sha256(m_Key1, sizeof(m_Key1), (char*)&uiMaskC, sizeof(uiMaskC), szMask, sizeof(szMask));
				//Do Xor
				for (uint32_t uiX = 0; uiX < m_uiSizePerEntry; uiX++)
				{
					pData[uiX] ^= szMask[uiX % SHA256_DIGEST_LENGTH];

				}
			}
			else
			{
				//Not Empty
				for (uint32_t uiX = 0; uiX < m_uiSizePerEntry; uiX++)
				{
					pData[uiX] ^= pXor[uiX % SHA256_DIGEST_LENGTH];

				}
			}

		}

		return uiRet;

	}

	char *SSECuckoo::_SetEntrys(char *pBucket, uint32_t &uiB)
	{
		for (uint32_t uiCur = 0; uiCur < m_uiB; uiCur++)
		{
			if (*(uint32_t*)(pBucket + (uiCur * m_uiSizePerEntry) + m_uiS) != m_uiFlag)
			{
				//Empty and set the flag
				*(uint32_t*)(pBucket + (uiCur * m_uiSizePerEntry) + m_uiS) = m_uiFlag;
				uiB = uiCur;
				return pBucket + (uiCur * m_uiSizePerEntry);
			}
		}
		//Full
		return NULL;
	}

	char *SSECuckoo::_GetEntrys(char *pBucket, char *pMask)
	{

		for (uint32_t uiCur = 0; uiCur < m_uiB; uiCur++)
		{
			char *pEntry = pBucket + uiCur * m_uiSizePerEntry;
			for (uint32_t uiX = 0; uiX < m_uiSizePerEntry; uiX++)
			{
				m_TempGet[uiX] = pEntry[uiX] ^ pMask[uiX];
			}
			if (*(uint32_t*)(m_TempGet + m_uiS) == m_uiFlag)
			{
				return m_TempGet;
			}
		}
		return NULL;

	}

}






#endif
