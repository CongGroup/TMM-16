#include <iostream>
#include <string>
#include <string.h>
#include <stdint.h>
#include <map>

#include "Digest.h"

using namespace std;

#define SSE_CUCKOO_KEY_LEN 256

namespace caravel{

class SSECuckoo
{

private:

    //Data structure Info

    char *m_pAll;

    //0 means the first  1 means the second
    char *m_Line[2];

    uint32_t *m_arTempXor;
    char *m_arMask;

    uint32_t m_uiLineCnt;
    uint32_t m_uiAllCnt;
    uint32_t m_uiSizePerBucket;
    uint32_t m_uiSizePerEntry;

    char *m_TempGet;

    uint32_t m_uiW;
    uint32_t m_uiB;
    uint32_t m_uiS;
    uint32_t m_uiMaxKick;

    uint32_t m_uiFlag;

    uint64_t m_ulMemSize;

    uint32_t m_uiCurKick;

    //Encryption Info

    char m_Key1[SHA256_DIGEST_LENGTH];
    char m_Key2[SHA256_DIGEST_LENGTH];
    char m_Key3[SHA256_DIGEST_LENGTH];
    

	//use map for temp store
	map<uint32_t, uint32_t> m_mapInsert;
	uint32_t m_uiUseMap;
    
    //Insert Info

    bool _Move(uint32_t uiLine, uint32_t uiI, uint32_t uiB, uint32_t uiKick);

    inline char *_SetEntrys(char *pBucket, uint32_t &uiB);
    inline char *_GetEntrys(char *pBucket, char *pMask);

public:
    SSECuckoo(void);
    ~SSECuckoo(void);

    uint32_t GetKick();

	uint32_t GetMapNum();

	uint32_t GetEmpty();

    uint64_t Size();
    uint64_t Size(uint32_t uiNum, double dLoad, uint32_t uiB, uint32_t uiS);

    //Init the data structure and return the real size
    //p is the pointer of Memory which can hold the Index
    //uiW is the count of Entry in one array
    //uiB is the count of Trunk in one Entry
    //uiS is the size of one Trunk
    //uiMaxKick is the max of kickout times.
    //return the memory size of the Index
    uint64_t Init(char *p, uint32_t uiW, uint32_t uiB, uint32_t uiS, uint32_t uiMaxKick, bool bReset = true);

    //uiNum is the number of all records. dPercent is the load.
    uint64_t Init(char *p, uint32_t uiNum, double dLoad, uint32_t uiB, uint32_t uiS, uint32_t uiMaxKick, bool bReset = true);



    //MasterKey will generate all the key. Just for Demo ^_^.
    void SetKey(string sMasterKey);

    //Put Key and Value
    //pKey is the pointer of fingerprint, uiKey is the length of pointer for safely using.
    //uiC is the counter of Data. A big data is split into uiC piece.
    //pVal is the pointer of value, uiVal is the length of pointer for safely using.
    // *important* : uiVal can not be greater than uiSize(the size of one Trunk).
    int Put(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parVal, uint32_t uiVal);

    bool Get(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parRet);

	uint32_t Insert(char *pKey, uint32_t uiKey, uint32_t uiCnt, char **parVal, uint32_t uiVal);

    int Encrypt();




};



}

