#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>

#include "../Caravel/TimeDiff.h"
#include "../Caravel/SemCtl.h"

#include "config.h"
#include "SecIndex.h"

using namespace std;
using namespace caravel;

int main(int argc, char **argv)
{
	if (argc != 9)
	{
		cout << "usage : ./" << argv[0] << " [BegTime] [RunTime] [uiLshNum] [uiRandSeed] [uiAllNum] [dLoad] [uiIndexB] [uiLshL]" << endl;
		return 0;
	}


	//Get the params from command line
    uint32_t uiBeg, uiTime, uiLshNum, uiSeed, uiAllNum, uiIndexB;
    double dLoad;
    uint32_t uiLshL;

	sscanf(argv[1], "%u", &uiBeg);
	sscanf(argv[2], "%u", &uiTime);
	sscanf(argv[3], "%u", &uiLshNum);
    sscanf(argv[4], "%u", &uiSeed);
    sscanf(argv[5], "%u", &uiAllNum);
    sscanf(argv[6], "%lf", &dLoad);
    sscanf(argv[7], "%u", &uiIndexB);
    sscanf(argv[8], "%u", &uiLshL);

    

	//Compute the params
	uint32_t uiCurTime;
	uint32_t uiBegTime = uiBeg;
	uint32_t uiEndTime = uiBegTime + uiTime;

    size_t sizeAllLen = sizeof(uint32_t) * uiLshL * uiLshNum;

    //Save for lsh values
    uint32_t *arLsh;
    ShmCtl::GetShm(&arLsh, DEF_SHMKEY_LSH, sizeAllLen);

    SecIndex secIndex;

    secIndex.SetKey(DEF_MASTER_KEY);

    uint32_t uiHugeNum = uiAllNum / dLoad + 1;

    secIndex.Init(DEF_SHMKEY_HUGEINDEX, uiLshL, uiHugeNum, uiIndexB, false);

    secIndex.SetMaxKickout(DEF_MAX_KICKOUT);
    secIndex.SetLshMetaData(arLsh);
	
    srand(uiSeed);

	uint32_t uiCnt = 0;
	while (true)
	{
		uiCurTime = time(NULL);

		if (uiCurTime < uiBegTime)
		{
			continue;
		}

		if (uiCurTime >= uiEndTime)
		{
			break;
		}

        uint32_t uiInsertID = rand() % (uiLshNum - 1) + 1;

        emInsertRet ret = secIndex.Insert(uiInsertID, rand() % uiLshL, uiInsertID);
		
        if (ret == emInsertRet::MaxKickout)
        {
            cout << "Max Kickout Appear!" << endl;
        }

		uiCnt++;
	}
	
	cout << uiCnt << endl;

	return 0;
}




