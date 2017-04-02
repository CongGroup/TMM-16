#ifndef __ISIC_H__
#define __ISIC_H__


//1 means colorful   0 means grey
#define DEF_READ_FLAG 0
#define DEF_ZERO_PERCENT 0.75

//#define DEF_IMG_W 256
//#define DEF_IMG_H 192
#define DEF_IMG_W 102//4
#define DEF_IMG_H 76//8

#define DEF_MATRIX_PATH "/dataset/ISIC_Mat"
#define DEF_CS_PATH "/dataset/ISIC_CS"
#define DEF_LSH_PATH "/dataset/ISIC_LSH"
#define DEF_PATHBUF_LEN 512

#define DEF_TOP_K 50

//0 ~ 4663
//1 ~ 4664
#define DEF_ISIC_MAXNUM 4665
#define DEF_ISIC_DIMENSION 5814
#define DEF_LSH_L 4
#define DEF_LSH_K 2
#define DEF_LSH_W 4



#endif // __ISIC_H__


#include <fstream>
#include <string.h>
#include <vector>

using namespace std;

uint32_t LoadOriginalMatrix(string sPath, double **p)
{
    fstream iff;
    iff.open(sPath, ios::in);

    uint32_t uiNum = DEF_IMG_W * DEF_IMG_H;

    *p = new double[uiNum];

    for (uint32_t uiCur = 0; uiCur < uiNum; uiCur++)
    {
        iff >> (*p)[uiCur];
    }

    return uiNum;

}


uint32_t LoadDoubleMatrix(string sPath, double **p)
{
    fstream iff;
    iff.open(sPath, ios::in);

    uint32_t uiNum;
    iff >> uiNum;

    *p = new double[uiNum];

    for (uint32_t uiCur = 0; uiCur < uiNum; uiCur++)
    {
        iff >> (*p)[uiCur];
    }

    return uiNum;

}


double ComputeAccuracy(vector<double> &vecQ, vector<double> &vecR, uint32_t uiK)
{
    return 0;
}
