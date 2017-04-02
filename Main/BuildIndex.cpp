#include <stdint.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <time.h>

#include "../Caravel/ShmCtl.h"
#include "../Caravel/Digest.h"
#include "../Caravel/BitConvert.h"
#include "../Caravel/SemCtl.h"
#include "../Caravel/TimeDiff.h"

#include "config.h"
#include "SecIndex.h"

using namespace std;
using namespace caravel;

int main(int argc, char** argv)
{
    uint32_t uiOffset;
    uint32_t uiLines;
    uint32_t uiZero;
    bool bZero = true;
    uint32_t uiAllNum;
    double dLoad;
    uint32_t uiB;
    uint32_t uiLshL;

    if (argc != 8)
    {
        cout << "Error Params : " << argv[0] << " {BeginLshOffset} {uiLines} {ifBeZero} {AllNum} {IndexLoad} {IndexB} {uiL}" << endl;
        return -1;
    }

    sscanf(argv[1], "%u", &uiOffset);
    sscanf(argv[2], "%u", &uiLines);
    sscanf(argv[3], "%u", &uiZero);
    sscanf(argv[4], "%u", &uiAllNum);
    sscanf(argv[5], "%lf", &dLoad);
    sscanf(argv[6], "%u", &uiB);
    sscanf(argv[7], "%u", &uiLshL);


    bZero = (uiZero == 0);

    if (bZero)
    {
        cout << "=========================================================" << endl << endl;
        cout << "Read Params List : " << endl;
        cout << "uiOffset = " << uiOffset << endl;
        cout << "uiLines = " << uiLines << endl;
        cout << "uiZero = " << uiZero << endl;
        cout << "uiAllNum = " << uiAllNum << endl;
        cout << "dLoad = " << dLoad << endl;
        cout << "uiB = " << uiB << endl;
        cout << "uiLshL = " << uiLshL << endl;
        cout << "---------------------------------------------------------" << endl << endl;
    }


    size_t sizeAllLen = sizeof(uint32_t) * uiLshL * uiAllNum;
    cout << sizeAllLen << endl;
    //Save for lsh values
    uint32_t *arLsh;
    ShmCtl::GetShm(&arLsh, DEF_SHMKEY_LSH, sizeAllLen);

    SecIndex secIndex;

    secIndex.SetKey(DEF_MASTER_KEY);

    uint32_t uiHugeNum = uiAllNum / dLoad + 1;

    secIndex.Init(DEF_SHMKEY_HUGEINDEX, uiLshL, uiHugeNum, uiB, bZero);

    secIndex.SetMaxKickout(DEF_MAX_KICKOUT);
    secIndex.SetLshMetaData(arLsh);

    if (bZero)
    {
        cout << "Finish memset all the index and bucket hash." << endl;
        return 0;
    }

    uint32_t uiEnd = uiOffset + uiLines;

    cout << "BegTime : " << (uint32_t)time(NULL) << endl;

    TimeDiff::DiffTimeInMicroSecond();

    for (uint32_t uiCur = uiOffset; uiCur < uiEnd; uiCur++)
    {

#ifdef DEF_NO_CACHE
        emInsertRet ret = secIndex.Insert_Nocache(uiCur, rand() % uiLshL, uiCur);
#else
        emInsertRet ret = secIndex.Insert(uiCur, rand() % uiLshL, uiCur);
#endif

        if (ret == emInsertRet::MaxKickout)
        {
            cout << "Max Kickout Appear!" << endl;
        }

        if (ret != emInsertRet::Success)
        {
            cout << "Insert ERR : " << ret << endl;
        }

        if ((uiCur - uiOffset) % (uiLines / 100) == 0)
        {
            cout << " [ " << uiOffset << " ] Build Index Percent => " << (uiCur - uiOffset) / (uiLines / 100) << "%" << endl;
            cout << "Current Kickout Num = " << secIndex.GetKickoutNum() << endl;
        }

    }

    uint32_t uiTimeCost = TimeDiff::DiffTimeInMicroSecond();

    cout << "Offset : " << uiOffset << " End : " << uiEnd << endl;
    cout << "All Kickback : " << secIndex.GetKickBackNum() << endl;
    cout << "EndTime : " << (uint32_t)time(NULL) << endl;
    cout << "Cost Microsecond : " << uiTimeCost << endl;

    return 0;
}


