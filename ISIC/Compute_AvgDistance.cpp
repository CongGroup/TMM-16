#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <map>
#include <vector>

#include <KL1pInclude.h>
#include "../Caravel/E2Lsh.h"
#include "../Caravel/C2Lsh.h"

using namespace std;
using namespace kl1p;
using namespace caravel;

#include "ISIC.h"

//#define DEF_OUTPUT_SAMPLE

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        cout << "Please use : " << argv[0] << " InputDoubleMatrixPath InputOriginalPath CheckID MaxID AccuracyTopK" << endl;
        cout << "e.g. : ./Compute_AvgDistance /dataset/ISIC_CS /dataset/ISIC_Mat 100 4665 10" << endl;
        return -1;
    }

    //1024 X 768



    uint32_t uiCheckID, uiMaxID, uiTopK;

    string srcPath(argv[1]);
    string orgPath(argv[2]);
    sscanf(argv[3], "%u", &uiCheckID);
    sscanf(argv[4], "%u", &uiMaxID);
    sscanf(argv[5], "%u", &uiTopK);

    double **parVal;
    parVal = new double*[uiMaxID];

    char szBuf[500];

    uint32_t uiD;

    cout << "Begin to read CS Matrix" << endl;

#ifdef DEF_OUTPUT_SAMPLE
    ofstream outf;
    outf.open("train.txt", ios::out);
#endif

    for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
    {
        snprintf(szBuf, sizeof(szBuf), "%s/%d", srcPath.c_str(), uiCur);
        string sTmpPath(szBuf);
        //cout << sTmpPath << endl;
        uiD = LoadDoubleMatrix(sTmpPath, parVal + uiCur);

#ifdef DEF_OUTPUT_SAMPLE
        for (uint32_t ui = 0; ui < uiD; ui++)
        {
            outf << parVal[uiCur][ui];
            if (ui != uiD - 1)
            {
                outf << "\t";
            }
        }
        outf << "\n";
        outf.flush();

#endif
    }

#ifdef DEF_OUTPUT_SAMPLE

    outf.close();

#endif

    double *arDistance;
    arDistance = new double[uiMaxID];

    double allDistance;

    uint32_t uiSmallA = 0;
    uint32_t uiSmallB = 0;

    cout << "uiD = " << uiD << endl;

    cout << "Begin to Compute the CS Matrix Distance" << endl;

    for (uint32_t uiCur = 0; uiCur < uiMaxID; uiCur++)
    {

        arDistance[uiCur] = C2Lsh::ComputeL2(parVal[uiCheckID], parVal[uiCur], uiD);
        allDistance += arDistance[uiCur];
        if (arDistance[uiCur] < 9.8)
        {
            uiSmallA++;
        }
        if (arDistance[uiCur] < 10)
        {
            //cout << arDistance[uiCur] << " By " << uiCur << endl;
            uiSmallB++;
        }

    }

    cout << "AVG Distance = " << allDistance / uiMaxID << endl;
    cout << "Distance < 9.8 = " << uiSmallA << endl;
    cout << "Distance < 10 = " << uiSmallB << endl;

    //Begin to with the Original Data

    cout << "Begin to Read the Original Data." << endl;

    double **parOrg;
    parOrg = new double*[uiMaxID];

    uint32_t uiOrgD;

    for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
    {
        snprintf(szBuf, sizeof(szBuf), "%s/%d", orgPath.c_str(), uiCur);
        string sTmpPath(szBuf);
        uiOrgD = LoadOriginalMatrix(sTmpPath, parOrg + uiCur);
    }

    double avgOrgDistance;

    double *arOrgDistance;
    arOrgDistance = new double[uiMaxID];

    cout << "Begin to Compute the Original Distance" << endl;

    for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
    {
        arOrgDistance[uiCur] = C2Lsh::ComputeL2(parOrg[uiCheckID], parOrg[uiCur], uiOrgD);
        avgOrgDistance += arOrgDistance[uiCur];
    }

    cout << "AVG Org Distance = " << avgOrgDistance / uiMaxID << endl;

    cout << "Begin to Compute the Accuracy" << endl;

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

    //Begin to compute Top K accuracy

}



