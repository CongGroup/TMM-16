#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <assert.h>
//getch
#include <termios.h>
#include <unistd.h>

#include "../Caravel/BukHash.h"
#include "../Caravel/ShmCtl.h"
#include "../Caravel/C2Lsh.h"
#include "../Caravel/E2Lsh.h"

//time compute
#include "../Caravel/TimeDiff.h"

#include "SecIndex.h"

//Config
#include "config.h"
#include "../ISIC/ISIC.h"

using namespace std;
using namespace caravel;

int getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int countLines(string sPath)
{
    ifstream ifs;
    ifs.open(sPath, ios::in);
    int iLines = 0;
    string strLines;
    if (ifs.fail())
    {
        return -1;
    }
    else
    {
        while (getline(ifs, strLines, '\n'))
        {
            iLines++;
        }
    }
    return iLines;
}

template<typename T>
T PrintAndGet(char *szMsg)
{
    T tRet;
    cout << szMsg << endl << endl;
    cin >> tRet;
    return tRet;
}


void TestMapThroughput(uint32_t uiCount)
{

	map<uint32_t, uint32_t> mapTest;
	TimeDiff::DiffTimeInMicroSecond();

	for (uint32_t uiCur = 0; uiCur < uiCount; uiCur++)
	{
		mapTest[uiCur] = uiCur;
	}

	uint32_t uiMs = TimeDiff::DiffTimeInMicroSecond();
	cout << "Insert " << uiCount << " into map cost " << uiMs << " microseconds" << endl;
}


void PrintCmdList() {
    system("clear");
    cout << endl << endl << "              Controller Command List" << endl << endl;
    cout << "  =====================================================" << endl << "  ||" << endl;
    cout << "  ||  Input Command Below :" << endl << "  ||" << endl;
    cout << "  =====================================================" << endl << "  ||" << endl;
    cout << "  || 0 Exit" << endl;
    cout << "  || 1 Process Data, Load huge lsh file to share memory. " << endl;
    cout << "  || 2 Load the Processed Data and informations from files." << endl;
    cout << "  =====================================================" << endl << "  ||" << endl;
    cout << "  || 3 [ Tool ] Turn LSH Text file to Binary File" << endl;
    cout << "  || 4 [ Tool ] Load BOW data" << endl;
    cout << "  || 5 [ Tool ] Collect the LSH map and print information." << endl;
    cout << "  || 6 [ Graph 7 ] Test Query latency." << endl;
    cout << "  || 7 [ Graph 3 ] Test Insert latency" << endl;
    cout << "  =====================================================" << endl << "  ||" << endl;


    cout << "  || 8 [ ISIC ] Process data from file and load lsh to share memory." << endl;
    cout << "  || 9 [ ISIC ] Query and compute accuracy." << endl;
    cout << "  || a " << endl;
    cout << "  || b " << endl;
    cout << "  || c " << endl;


    cout << "  || 5 " << endl;
    cout << "  || 6 " << endl;
    cout << "  || 7 " << endl;
    cout << "  || 8 " << endl << "  ||" << endl;


    cout << "  =====================================================" << endl << "  ||" << endl;
    cout << "  || A [Graph 1] Test create index using huge lsh" << endl;
    cout << "  || B [ Tool ] Attach created index" << endl;
    cout << "  || C [ Tool ] Print index bucket hash state" << endl;
    cout << "  || D [ Tool ] Test index query" << endl;
    cout << "  || E [ Tool ] Print ID's LSH" << endl;
    cout << "  || F [ Tool ] Encrypt Index" << endl;
    cout << "  || G [ Tool ] Build multiply copy index using lsh in share memory." << endl;



    cout << "  =====================================================" << endl << endl;

    return;
}

int main(int argc, char **argv)
{
	/*
	TestMapThroughput(100);
	TestMapThroughput(1000);
	TestMapThroughput(10000);
	TestMapThroughput(100000);
	TestMapThroughput(1000000);
	TestMapThroughput(10000000);
	TestMapThroughput(100000000);
	*/


    srand(0);

    //Save for meta data
    double **arMetaVal;

    //Save for lsh values
    uint32_t *arLsh;

    uint64_t ulAllNum;
    uint32_t uiLshL;
    double dLshW;
    uint32_t uiDataDimension;
    uint32_t uiUseDimension;

    uint32_t uiB;

    //RAW DATA


    //index setting
    uint32_t uiBuildIndexNum;

    //Enc Index

    SecIndex secIndex;
    secIndex.SetKey(DEF_MASTER_KEY);

    system("clear");
    int iCmd;
    do {
        PrintCmdList();
        iCmd = getch();
        switch (iCmd)
        {
        case 49:
        {
            //1 Process Data, Load huge lsh file to share memory

            string strPath = PrintAndGet<string>("Please input the Path of the lsh file [ Then Press Enter ]\n e.g.  LSH_PURE_ARR_R0002_L20_K8.dat \n 1M_L20_R003 ");

            ulAllNum = PrintAndGet<uint64_t>("Please input the number of the dataset. [ Then Press Enter ]\n e.g.  LSH_PURE_ARR_R0002_L20_K8.dat is 1298600014. \n 1M_L20_R003 is 1000000");

            uiLshL = PrintAndGet<uint32_t>("Please input the L of LSH function. [ Then Press Enter ]\n e.g.  LSH_PURE_ARR_R0002_L20_K8.dat is 20. \n 1M_L20_R003 is 20");

            cout << "Begin to Load data from [ " << strPath.c_str() << " ]" << endl;
            cout << "The number is [ " << ulAllNum << " ]" << endl;
            cout << "The L is " << uiLshL << endl;

            size_t sizeAllLen = sizeof(uint32_t) * uiLshL * ulAllNum;
            cout << "All the memory is " << sizeAllLen << endl;
            cout << "Begin to get share memory" << endl;

            ShmCtl::GetShm(&arLsh, DEF_SHMKEY_LSH, sizeAllLen);
            memset(arLsh, 0, sizeAllLen);

            //Begin to read lsh file to share memory
            ifstream myfile;
            myfile.open(strPath.c_str(), ios::in | ios::binary | ios::ate);

            size_t siLen = myfile.tellg();
            if (siLen != sizeAllLen)
            {
                cout << "Error The size of memory is not equal to the size of file." << siLen << endl;
                cout << "Compute size = " << sizeAllLen << endl;
                //return -1;
            }

            cout << "Finish checking the size : " << siLen << endl << "Begin to read file into shm." << endl;

            myfile.seekg(ios::beg);

            cout << "Begin to read data." << endl;

            for (uint64_t ulCur = 0; ulCur < ulAllNum; ulCur++)
            {
                for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
                {
                    size_t sizeOff = ulCur * uiLshL + uiL;
                    myfile.read((char*)(arLsh + sizeOff), sizeof(uint32_t));
                }

                if (ulCur % (ulAllNum / 100) == 0)
                {
                    cout << "Print load lsh percent => " << ulCur / (ulAllNum / 100) << "%" << endl;
                }
            }

            cout << "Finish load the data." << endl;
            cout << "Check the data e.g. No. 100 : " << endl;
            for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
            {
                cout << arLsh[100 * uiLshL + uiL] << " ";
            }

            break;
        }
        case 50:
        {
            //2
            ulAllNum = PrintAndGet<uint32_t>("Please input the number of the dataset. [ Then Press Enter ]\n e.g.  LSH_PURE_ARR_R0002_L20_K8.dat is 1298600014.");
            uiLshL = PrintAndGet<uint32_t>("Please input the L of LSH function. [ Then Press Enter ]\n e.g.  LSH_PURE_ARR_R0002_L20_K8.dat is 20.");

            cout << "The number is [ " << ulAllNum << " ]" << endl;
            cout << "The L is " << uiLshL << endl;

            size_t sizeAllLen = sizeof(uint32_t) * uiLshL * ulAllNum;
            cout << "All the memory is " << sizeAllLen << endl;
            cout << "Begin to attach share memory" << endl;

            ShmCtl::GetShm(&arLsh, DEF_SHMKEY_LSH, sizeAllLen);

            cout << "Finish Attach." << endl;

            uint32_t *pLsh = arLsh + (100 * uiLshL);
            for (uint32_t uiCur = 0; uiCur < uiLshL; uiCur++)
            {
                cout << pLsh[uiCur] << " ";
            }

            break;
        }
        case 51:
        {
            //3 [ Tool ] Turn LSH Text file to Binary File

            string strPath = PrintAndGet<string>("Please input the Path of the lsh file [ Then Press Enter ]\n e.g.  lsh-L100-R003");

            ulAllNum = PrintAndGet<uint64_t>("Please input the number of the dataset. [ Then Press Enter ]\n e.g.  lsh-L100-R003 is 1000000.");

            uiLshL = PrintAndGet<uint32_t>("Please input the L of LSH function. [ Then Press Enter ]\n e.g.  lsh-L100-R003 is 100.");

            uint32_t uiOutL = PrintAndGet<uint32_t>("Please input the L of LSH function for write binary. [ Then Press Enter ]\n e.g.  commonly use 20.");

            ifstream myfile;
            myfile.open(strPath.c_str());

            ofstream off;
            off.open("OUT_LSH_BINARY.dat", ios::binary);

            cout << "Begin to read and write data." << endl;

            for (uint64_t ulCur = 0; ulCur < ulAllNum; ulCur++)
            {
                for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
                {
                    size_t sizeOff = ulCur * uiLshL + uiL;
                    uint32_t uiTmp;
                    myfile >> uiTmp;

                    if (uiL < uiOutL)
                    {
                        off.write((char*)&uiTmp, sizeof(uint32_t));
                    }

                    if (ulCur == 100)
                    {
                        cout << uiTmp << " ";
                    }

                }

                if (ulCur % (ulAllNum / 100) == 0)
                {
                    cout << "Print load lsh percent => " << ulCur / (ulAllNum / 100) << "%" << endl;
                }
            }

            myfile.close();
            off.close();
            cout << "Finish write binary file." << endl;

            break;
        }
        case 52:
        {
            //4 [ Tool ] Load BOW data



            break;


        }
        case 53:
        {
            //5 [ Tool ] Collect the LSH map and print information.

            map<uint32_t, uint32_t> arMap[20];

            for (uint32_t uiCur = 0; uiCur < ulAllNum; uiCur++)
            {
                for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
                {
                    arMap[uiL][arLsh[uiCur * uiLshL + uiL]]++;
                }
            }

            for (uint32_t uiCur = 0; uiCur < uiLshL; uiCur++)
            {
                uint32_t uiCount = 0;
                for (map<uint32_t, uint32_t>::iterator it = arMap[uiCur].begin(); it != arMap[uiCur].end(); it++)
                {
                    uiCount++;
                }
                cout << "MAP[" << uiCur << "] = " << uiCount << endl;
            }

            break;
        }
        case 54:
        {
            //6 Test Query latency.

            uint32_t *arQueryRet;
            uint32_t uiMaxRetNum = uiLshL * uiB;
            arQueryRet = new uint32_t[uiMaxRetNum];
            memset(arQueryRet, 0, uiMaxRetNum * sizeof(uint32_t));

            uint32_t uiQueryID = PrintAndGet<uint32_t>("Please input your query ID.");
            uint32_t uiTestNum = PrintAndGet<uint32_t>("Please input your query times.");

            uint32_t uiRetNum = 0;

            TimeDiff::DiffTimeInMicroSecond();
            for (uint32_t uiCur = 0; uiCur < uiTestNum; uiCur++)
            {
                secIndex.Query(uiQueryID + uiCur, arQueryRet, uiRetNum);
            }

            uint32_t uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
            cout << "Test " << uiTestNum << " Cost Time MicroSeconds = " << uiTimeCost << endl;

            delete[] arQueryRet;

            break;
        }
        case 55:
        {
            //7 [ Graph 3 ] Test Insert latency

            uint32_t uiQueryID = PrintAndGet<uint32_t>("Please input your query ID.");
            uint32_t uiTestNum = PrintAndGet<uint32_t>("Please input your query times.");

            TimeDiff::DiffTimeInMicroSecond();
            for (uint32_t uiCur = 0; uiCur < uiTestNum; uiCur++)
            {
                emInsertRet ret = secIndex.Insert(uiQueryID + uiCur, rand() % uiLshL, uiQueryID + uiCur);
                if (ret == emInsertRet::MaxKickout)
                {
                    cout << "Max Kickout Appear!" << endl;
                }
            }

            uint32_t uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
            cout << "Test " << uiTestNum << " Cost Time MicroSeconds = " << uiTimeCost << endl;

            break;
        }
        case 56:
        {
            // 8 [ ISIC ] Process data from file and load lsh to share memory.
            string sPath = PrintAndGet<string>("Please input the LSH Text file     \n /dataset/ISIC_LSH/L10K6");
            ulAllNum = PrintAndGet<uint32_t>("Please input the number of the dataset. [ Then Press Enter ]\n e.g. ISIC is 4664");
            uiLshL = PrintAndGet<uint32_t>("Please input the L of LSH function. [ Then Press Enter ]\n e.g.  ISIC is 4.");

            cout << "The Path is [ " << sPath << " ] " << endl;
            cout << "The number is [ " << ulAllNum << " ]" << endl;
            cout << "The L is " << uiLshL << endl;

            size_t sizeAllLen = sizeof(uint32_t) * uiLshL * ulAllNum;
            cout << "All the memory is " << sizeAllLen << endl;
            cout << "Begin to attach share memory" << endl;

            ShmCtl::GetShm(&arLsh, DEF_SHMKEY_LSH, sizeAllLen);

            ifstream iff;
            iff.open(sPath, ios::in);


            for (uint32_t uiCur = 1; uiCur <= ulAllNum; uiCur++)
            {
                uint32_t *pLsh = arLsh + (uiCur * uiLshL);
                for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
                {
                    iff >> pLsh[uiL];
                }
            }

            break;
        }
        case 57:
        {
            // 9 [ ISIC ] Query and compute accuracy.

            uint32_t *arQueryRet;
            uint32_t uiMaxRetNum = uiLshL * uiB;
            arQueryRet = new uint32_t[uiMaxRetNum];
            memset(arQueryRet, 0, uiMaxRetNum * sizeof(uint32_t));

            uint32_t uiQueryID = PrintAndGet<uint32_t>("Please input your query ID.");
            //the file is from 1 ~ 4664
            uint32_t uiFileQueryID = uiQueryID;

            uint32_t uiRetNum = 0;
            secIndex.Query(uiQueryID, arQueryRet, uiRetNum);

            for (uint32_t uiCur = 0; uiCur < uiRetNum; uiCur++)
            {
                if (arQueryRet[uiCur] == uiQueryID)
                {
                    cout << " [ " << arQueryRet[uiCur] << " ] ";
                }
                else
                {
                    cout << arQueryRet[uiCur] << " ";
                }
            }

            cout << "All return " << uiRetNum << " results." << endl;

            map<uint32_t, uint32_t> mapDistinct;
            for (uint32_t uiCur = 0; uiCur < uiRetNum; uiCur++)
            {
                mapDistinct[arQueryRet[uiCur]]++;
            }
            uint32_t uiCount = 0;
            for (map<uint32_t, uint32_t>::iterator it = mapDistinct.begin(); it != mapDistinct.end(); it++)
            {
                uiCount++;
            }

            delete[] arQueryRet;
            uiMaxRetNum = uiCount;
            arQueryRet = new uint32_t[uiMaxRetNum];
            uiCount = 0;
            for (map<uint32_t, uint32_t>::iterator it = mapDistinct.begin(); it != mapDistinct.end(); it++)
            {
                arQueryRet[uiCount++] = it->first;
            }

            cout << "Begin to compute the accuracy." << endl;

            // ************************************        Read CS Matrix and compute the distance   **********************
			//compare with all cs data.
            double **parVal;

            //store the distance
            uint32_t uiMaxID = DEF_ISIC_MAXNUM;
            parVal = new double*[uiMaxID];

            //Dimension
            uint32_t uiD;

            //Read cs data from path
            string strCSPath = DEF_CS_PATH;
            for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
            {
                char szBuf[DEF_PATHBUF_LEN];
                snprintf(szBuf, sizeof(szBuf), "%s/%d", strCSPath.c_str(), uiCur);
                string sTmpPath(szBuf);
                //cout << sTmpPath << endl;
                uiD = LoadDoubleMatrix(sTmpPath, parVal + uiCur);
            }

            cout << "uiD = " << uiD << endl;
            assert(uiD == DEF_ISIC_DIMENSION);

            //Compute distance to all point from query point
            double *arDistance;
            arDistance = new double[uiMaxID];
            double allDistance;

            cout << "Begin to Compute the CS Matrix Distance" << endl;
            //Compute L2 Distance
            for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
            {
                arDistance[uiCur] = C2Lsh::ComputeL2(parVal[uiFileQueryID], parVal[uiCur], uiD);
                allDistance += arDistance[uiCur];
            }

            cout << "The point ID = " << uiFileQueryID << " AVG distance to all is = " << allDistance / (uiMaxID - 1) << endl;

            // ************************************   Read Original Matrix and compute the distance   **********************

            //Begin to with the Original Data

            cout << "Begin to Read the Original Data." << endl;

            //Store the original raw data
            double **parOrg;
            parOrg = new double*[uiMaxID];

            //the dimension of the raw data
            uint32_t uiOrgD;

            string strOriginalPath = DEF_MATRIX_PATH;

            for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
            {
                char szBuf[DEF_PATHBUF_LEN];
                snprintf(szBuf, sizeof(szBuf), "%s/%d", strOriginalPath.c_str(), uiCur);
                string sTmpPath(szBuf);
                uiOrgD = LoadOriginalMatrix(sTmpPath, parOrg + uiCur);
            }

            cout << "uiOrgD = " << uiOrgD << endl;

            //Average Original Distance from Query point to all point
            double avgOrgDistance;

            //store the original data distance
            double *arOrgDistance;
            arOrgDistance = new double[uiMaxID];

            cout << "Begin to Compute the Original Distance" << endl;

            for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
            {
                arOrgDistance[uiCur] = C2Lsh::ComputeL2(parOrg[uiFileQueryID], parOrg[uiCur], uiOrgD);
                avgOrgDistance += arOrgDistance[uiCur];
            }

            cout << "AVG Org Distance = " << avgOrgDistance / (uiMaxID - 1) << endl;

            // ************************************   Begin to compute the accuracy of two source   **********************
			//compare with query cs data.

            uint32_t uiTopK = DEF_TOP_K;

            map<double, vector<uint32_t> > mapCS, mapOrg;

            for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
            {
                mapCS[arDistance[uiCur]].push_back(uiCur);
                mapOrg[arOrgDistance[uiCur]].push_back(uiCur);
            }

            vector<double> vecCS, vecOrg;

            cout << "For CS Matrix" << endl;

            uint32_t uiCnt = 0;
            bool bOut = false;
            for (map<double, vector<uint32_t> >::iterator it = mapCS.begin(); it != mapCS.end(); it++)
            {
                if (it->first == 0)
                {
                    cout << "Zero = " << it->second[0] << endl;
                    continue;
                }

                for (vector<uint32_t>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
                {
                    vecCS.push_back(it->first);
                    cout << "NO = " << *itt << " Distance = " << it->first << endl;
                    uiCnt++;
                    if (uiCnt == uiTopK)
                    {
                        bOut = true;
                        break;
                    }
                }

                if (bOut)
                {
                    break;
                }
            }

            cout << "For Original Matrix" << endl;

            uiCnt = 0;
            bOut = false;
            for (map<double, vector<uint32_t> >::iterator it = mapOrg.begin(); it != mapOrg.end(); it++)
            {
                if (it->first == 0)
                {
                    cout << "Zero = " << it->second[0] << endl;
                    continue;
                }

                for (vector<uint32_t>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
                {
                    vecOrg.push_back(it->first);
                    cout << "NO = " << *itt << " Distance = " << it->first << endl;
                    uiCnt++;
                    if (uiCnt == uiTopK)
                    {
                        bOut = true;
                        break;
                    }
                }
                if (bOut)
                {
                    break;
                }
            }


            ////////////////////////////////////////////For the all soft
            cout << "----------------------------------------------------------------------------------" << endl;

            map<double, vector<uint32_t> > mapQueryCS, mapQueryOrg;

            for (uint32_t uiCur = 0; uiCur < uiMaxRetNum; uiCur++)
            {
                uint32_t uiCurID = arQueryRet[uiCur];
                mapQueryCS[arDistance[uiCurID]].push_back(uiCurID);
                mapQueryOrg[arOrgDistance[uiCurID]].push_back(uiCurID);
            }

            vector<double> vecQueryCS, vecQueryOrg;
            cout << "For CS Query Matrix" << endl;

            uiCnt = 0;
            bOut = false;
            for (map<double, vector<uint32_t> >::iterator it = mapQueryCS.begin(); it != mapQueryCS.end(); it++)
            {
                if (it->first == 0)
                {
                    cout << "Zero = " << it->second[0] << endl;
                    continue;
                }

                for (vector<uint32_t>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
                {
                    vecQueryCS.push_back(it->first);
                    cout << "NO = " << *itt << " Distance = " << it->first << endl;
                    uiCnt++;
                    if (uiCnt == uiTopK)
                    {
                        bOut = true;
                        break;
                    }
                }

                if (bOut)
                {
                    break;
                }
            }

            cout << "For Original Query Matrix" << endl;


            uiCnt = 0;
            bOut = false;
            for (map<double, vector<uint32_t> >::iterator it = mapQueryOrg.begin(); it != mapQueryOrg.end(); it++)
            {
                if (it->first == 0)
                {
                    cout << "Zero = " << it->second[0] << endl;
                    continue;
                }

                for (vector<uint32_t>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
                {
                    vecQueryOrg.push_back(it->first);
                    cout << "NO = " << *itt << " Distance = " << it->first << endl;
                    uiCnt++;
                    if (uiCnt == uiTopK)
                    {
                        bOut = true;
                        break;
                    }
                }
                if (bOut)
                {
                    break;
                }
            }

            cout << "------------------------------------------------" << endl;

            double dCSPrecision = 0;
            double dOrgPrecision = 0;
            for (uint32_t uiCur = 0; uiCur < uiTopK; uiCur++)
            {
                dCSPrecision += vecCS[uiCur] / vecQueryCS[uiCur];
                dOrgPrecision += vecOrg[uiCur] / vecQueryOrg[uiCur];

                if (uiCur % 10 == 9)
                {
                    cout << "uiCur == " << uiCur << endl;
                    cout << "dCSPrecision = " << dCSPrecision / (uiCur + 1) << endl;
                    cout << "dOrgPrecision = " << dOrgPrecision / (uiCur + 1) << endl;
                    cout << endl;
                }

            }

            cout << "************************************************************" << endl;

            cout << "dCSPrecision = " << dCSPrecision / uiTopK << endl;
            cout << "dOrgPrecision = " << dOrgPrecision / uiTopK << endl;

            cout << "************************************************************" << endl;

            break;
        }
        case 97:
        {
            //a
            break;
        }
        case 98:
        {
            //b
            break;
        }
        case 99:
        {
            //c


            break;
        }
        case 65:
        {
            //A Test create index using huge lsh

            double dLoad = PrintAndGet<double>("Please input your index load.  e.g. 0.8");

            uint32_t uiInsertNum = PrintAndGet<uint32_t>("Please input the number of insert.");

            uint32_t uiHugeNum = uiInsertNum / dLoad + 1;

            cout << "Block Size = " << sizeof(IndexBlock) << endl;

            uiB = PrintAndGet<uint32_t>("Please input your prober number.");
            uint64_t ulAllMemory = secIndex.Init(DEF_SHMKEY_HUGEINDEX, uiLshL, uiHugeNum, uiB, true);
            cout << "ulAllMemory = " << ulAllMemory << "   uiHugeNum = " << uiHugeNum << endl;

            //uint32_t uiInsertNumofL = PrintAndGet<uint32_t>("Please input your insert lsh-id pair per item.");
            secIndex.SetMaxKickout(DEF_MAX_KICKOUT);
            secIndex.SetLshMetaData(arLsh);

            uint32_t uiMaxKickoutNum = 0;

            TimeDiff::DiffTimeInMicroSecond();

            for (uint32_t uiCur = 1; uiCur < uiInsertNum; uiCur++)
            {

#ifdef DEF_NO_CACHE
                emInsertRet ret = secIndex.Insert_Nocache(uiCur, rand() % uiLshL, uiCur);
#else
                emInsertRet ret = secIndex.Insert(uiCur, rand() % uiLshL, uiCur);
#endif
                if (ret == emInsertRet::MaxKickout)
                {
                    uiMaxKickoutNum++;
                    //cout << "Max Kickout Appear!" << endl;
                }

                if (uiCur % (uiInsertNum / 100) == 0)
                {
                    //cout << "Build Index Percent => " << uiCur / (uiInsertNum / 100) << "%" << endl;
                    //cout << "Current Kickout Num = " << secIndex.GetKickoutNum() << endl;
                }

                if (ret != emInsertRet::Success)
                {
                    cout << "Insert ERR : " << ret << endl;
                }

            }

			uint32_t uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
			cout << "Build Index Cost time : " << uiTimeCost << endl;
            cout << "uiMaxKickoutNum = " << uiMaxKickoutNum << endl;
            cout << "All Kickback Num = " << secIndex.GetKickBackNum() << endl;
			cout << "The max kickout is " << secIndex.GetInsertMaxKickout() << endl;

            uint32_t uiCmdEnc = PrintAndGet<uint32_t>("Press 0 to encrypt the index.");
            if (uiCmdEnc == 0)
            {
                cout << "Begin to encrypt the index." << endl;
                TimeDiff::DiffTimeInMicroSecond();

                secIndex.EncryptIndex();

                uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
                cout << "Encrypt Index Cost time : " << uiTimeCost << endl;

            }

            break;
        }
        case 66:
        {
            //B Attach Created Index

            double dLoad = PrintAndGet<double>("Please input your index load.  e.g. 0.8");

            uint32_t uiInsertNum = PrintAndGet<uint32_t>("Please input the number of insert.");

            uint32_t uiHugeNum = uiInsertNum / dLoad + 1;

            cout << "Attach Number " << uiHugeNum << endl;

            uiB = PrintAndGet<uint32_t>("Please input your prober number.");
            secIndex.Init(DEF_SHMKEY_HUGEINDEX, uiLshL, uiHugeNum, uiB, false);

            //uint32_t uiInsertNumofL = PrintAndGet<uint32_t>("Please input your insert lsh-id pair per item.");
            secIndex.SetMaxKickout(DEF_MAX_KICKOUT);
            secIndex.SetLshMetaData(arLsh);

            cout << "Attach Index Success." << endl;

            break;

        }
        case 67:
        {
            //C Print index bucket hash state

            for (uint32_t uiCur = 0; uiCur < uiLshL; uiCur++)
            {
                secIndex.ShowBukState(uiCur);
            }

            break;
        }
        case 68:
        {
            //D Test index query


            uint32_t *arQueryRet;
            uint32_t uiMaxRetNum = uiLshL * uiB;
            arQueryRet = new uint32_t[uiMaxRetNum];
            memset(arQueryRet, 0, uiMaxRetNum * sizeof(uint32_t));

            uint32_t uiQueryID = PrintAndGet<uint32_t>("Please input your query ID.");

            uint32_t uiRetNum = 0;
            secIndex.Query(uiQueryID, arQueryRet, uiRetNum);

            for (uint32_t uiCur = 0; uiCur < uiRetNum; uiCur++)
            {
                if (arQueryRet[uiCur] == uiQueryID)
                {
                    cout << " [ " << arQueryRet[uiCur] << " ] ";
                }
                else
                {
                    cout << arQueryRet[uiCur] << " ";
                }
            }

            cout << "All return " << uiRetNum << " results." << endl;


            delete[] arQueryRet;

            break;
        }
        case 69:
        {
            //E Print ID's LSh
            do
            {
                uint32_t uiQueryID = PrintAndGet<uint32_t>("Please input your Query ID.");
                if (uiQueryID == 0)
                {
                    break;
                }
                for (uint32_t uiCur = 0; uiCur < uiLshL; uiCur++)
                {
                    cout << arLsh[uiQueryID * uiLshL + uiCur] << " ";
                }
                cout << endl;

            } while (true);
            break;
        }
        case 70:
        {
            //F Encrypt Index

            secIndex.EncryptIndex();
            cout << "Finish Encrypt." << endl;
            break;

        }
        case 71:
        {
            // G[Tool] Build multiply copy index using lsh in share memory.

            double dLoad = PrintAndGet<double>("Please input your index load.  e.g. 0.8");

            uint32_t uiInsertNum = PrintAndGet<uint32_t>("Please input the number of insert.");

            uint32_t uiCopy = PrintAndGet<uint32_t>("Please input the copys.");

            uint32_t uiHugeNum = (uiInsertNum * uiCopy) / dLoad + 1;

            cout << "Block Size = " << sizeof(IndexBlock) << endl;

            uiB = PrintAndGet<uint32_t>("Please input your prober number.");
            uint64_t ulAllMemory = secIndex.Init(DEF_SHMKEY_HUGEINDEX, uiLshL, uiHugeNum, uiB, true);
            cout << "ulAllMemory = " << ulAllMemory << "   uiHugeNum = " << uiHugeNum << endl;

            //uint32_t uiInsertNumofL = PrintAndGet<uint32_t>("Please input your insert lsh-id pair per item.");
            secIndex.SetMaxKickout(DEF_MAX_KICKOUT);
            secIndex.SetLshMetaData(arLsh);

            uint32_t uiMaxKickoutNum = 0;

            TimeDiff::DiffTimeInMicroSecond();

            for (uint32_t uiCur = 1; uiCur < uiInsertNum; uiCur++)
            {

                for (uint32_t uiC = 0; uiC < uiCopy; uiC++)
                {
#ifdef DEF_NO_CACHE
                    emInsertRet ret = secIndex.Insert_Nocache(uiCur, rand() % uiLshL, uiCur);
#else
                    emInsertRet ret = secIndex.Insert(uiCur, rand() % uiLshL, uiCur);
#endif
                    if (ret == emInsertRet::MaxKickout)
                    {
                        uiMaxKickoutNum++;
                        cout << "Max Kickout Appear!" << endl;
                    }

                    if (ret != emInsertRet::Success)
                    {
                        cout << "Insert ERR : " << ret << endl;
                    }

                }

                if (uiCur % (uiInsertNum / 100) == 0)
                {
                    cout << "Build Index Percent => " << uiCur / (uiInsertNum / 100) << "%" << endl;
                    cout << "Current Kickout Num = " << secIndex.GetKickoutNum() << endl;
                }

                

            }

            uint32_t uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
            cout << "uiMaxKickoutNum = " << uiMaxKickoutNum << endl;
            cout << "All Kickback Num = " << secIndex.GetKickBackNum() << endl;
            cout << "Build Index Cost time : " << uiTimeCost << endl;

            uint32_t uiCmdEnc = PrintAndGet<uint32_t>("Press 0 to encrypt the index.");
            if (uiCmdEnc == 0)
            {
                cout << "Begin to encrypt the index." << endl;
                TimeDiff::DiffTimeInMicroSecond();

                secIndex.EncryptIndex();

                uiTimeCost = TimeDiff::DiffTimeInMicroSecond();
                cout << "Encrypt Index Cost time : " << uiTimeCost << endl;

            }

            break;

            break;


        }
        default:
        {
            continue;
            break;
        }
        };

        cout << "Press [Enter] to continue..." << endl;
        getch();
        getch();

    } while (48 != iCmd);

    cout << "Good Bye !" << endl;

    return 0;
}
