#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <KL1pInclude.h>
#include "../Caravel/E2Lsh.h"

using namespace std;
using namespace kl1p;

#include "ISIC.h"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "Please use : " << argv[0] << " InputCompressedMatrixPath OutputLSHPath LshL LshK" << endl;
        cout << "e.g. " << argv[0] << " /dataset/ISIC_CS /dataset/ISIC_LSH/cs.lsh 10 6" << endl;
        return -1;
    }

    //1024 X 768

    uint32_t uiLshL;
    uint32_t uiLshK;

    string srcPath(argv[1]);
    string desPath(argv[2]);
    sscanf(argv[3], "%u", &uiLshL);
    sscanf(argv[4], "%u", &uiLshK);


    E2Lsh e2lsh;
    e2lsh.InitLSH(10, DEF_ISIC_DIMENSION, uiLshL, uiLshK, DEF_LSH_W);

    double **parVal;

    uint32_t uiMaxID = DEF_ISIC_MAXNUM;
    parVal = new double*[uiMaxID];

    char szBuf[500];

    uint32_t uiD = DEF_ISIC_DIMENSION;

    for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
    {
        snprintf(szBuf, sizeof(szBuf), "%s/%d", srcPath.c_str(), uiCur);
        string sTmpPath(szBuf);
        //cout << sTmpPath << endl;
        LoadDoubleMatrix(sTmpPath, parVal + uiCur);
    }

    uint32_t **arLSH;
    arLSH = new uint32_t*[uiMaxID];

    cout << "Begin to Compute LSH" << endl;

    ofstream off;
    off.open(desPath, ios::out);

    for (uint32_t uiCur = 1; uiCur < uiMaxID; uiCur++)
    {
        arLSH[uiCur] = new uint32_t[uiLshL];

        e2lsh.ComputeLSH(parVal[uiCur], arLSH[uiCur]);

        for (uint32_t uiL = 0; uiL < uiLshL; uiL++)
        {
            off << arLSH[uiCur][uiL];

            if (uiL != uiLshL - 1)
            {
                off << "\t";
            }
        }

        off << "\n";

    }

    off.close();

}



