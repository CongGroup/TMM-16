#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include "../Caravel/C2Lsh.h"

using namespace std;
using namespace caravel;


#include "ISIC.h"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "Please use : " << argv[0] << " InputImageMatrix InputRecoverMatrix QID ID" << endl;
        cout << "e.g. " << argv[0] << " /dataset/ISIC_Mat /dataset/ISIC_Recover 74 176" << endl;
        return -1;
    }

    //1024 X 768

    string strMatPath(argv[1]);
    string strRecoverPath(argv[2]);
    uint32_t uiQID;
    uint32_t uiID;
    sscanf(argv[3], "%u", &uiQID);
    sscanf(argv[4], "%u", &uiID);

    char szBuf[DEF_PATHBUF_LEN];




    //cout << "Begin to read Image Matrix" << endl;

    double *arOriginalMat;

    snprintf(szBuf, sizeof(szBuf), "%s/%d", strMatPath.c_str(), uiID);
    string sTmpPath(szBuf);
    uint32_t uiOriginalD = LoadOriginalMatrix(sTmpPath, &arOriginalMat);

    //cout << "Begin to read Recovered Matrix" << endl;

    double *arRecoverMat;

    snprintf(szBuf, sizeof(szBuf), "%s/%d", strRecoverPath.c_str(), uiID);
    sTmpPath.assign(szBuf);
    uint32_t uiRecoverD = LoadOriginalMatrix(sTmpPath, &arRecoverMat);


    //cout << "Begin to Compute the value." << endl;

    if (uiRecoverD != uiOriginalD)
    {
        cout << "uiRecoverD != uiOriginalD !!!" << endl;
        return;
    }

    double *arZeroMat;
    arZeroMat = new double[uiRecoverD];
    memset(arZeroMat, 0, sizeof(double) * uiRecoverD);

    double dDistance = C2Lsh::ComputeL2(arOriginalMat, arRecoverMat, uiRecoverD);
    double dZeroDistance = C2Lsh::ComputeL2(arOriginalMat, arZeroMat, uiRecoverD);

    cout << uiID <<" >> Error on Value = dDistance / dZeroDistance = " << dDistance / dZeroDistance << endl;

}



