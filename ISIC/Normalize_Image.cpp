#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <opencv2/opencv.hpp>

#include <map>

using namespace std;
using namespace cv;

#include "ISIC.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cout << "Please use : " << argv[0] << " InputImagePath OutputImagePath OutputMatrix" << endl;
        return -1;
    }

    //1024 X 768

    string srcPath(argv[1]);
    string desPath(argv[2]);
    string matPath(argv[3]);

    Mat srcImg = imread(srcPath, DEF_READ_FLAG);
    Mat dstImg(DEF_IMG_H, DEF_IMG_W, srcImg.type());
    resize(srcImg, dstImg, dstImg.size(), 0, 0, INTER_LINEAR);
    imwrite(desPath, dstImg);

    cout << "srcImg.type() = " << srcImg.type() << endl;


    //Process the Image
    uint32_t uiAllPoint = dstImg.rows * dstImg.cols;
    double dZeroPercent = DEF_ZERO_PERCENT;
    uint32_t uiAllZeroPoint = uiAllPoint * dZeroPercent;

    map<uint32_t, uint32_t> mapImgColor;
    for (uint32_t i = 0; i < dstImg.rows; i++)
    {
        for (uint32_t j = 0; j < dstImg.cols; j++)
        {
            uint8_t ubVal = dstImg.at<uint8_t>(i, j);
            mapImgColor[ubVal]++;
        }
    }

    uint32_t uiFrom;
    uint32_t uiTo;

    cout << "uiAllZeroPoint = " << uiAllZeroPoint << endl;

    uint32_t uiCanZeroNum;
    //Find the fit zone
    for (uint32_t uiW = 1; uiW < 256; uiW++)
    {
        map<uint32_t, uint32_t> mapCombineColor;
        for (uint32_t uiOff = 0; uiOff < uiAllPoint; uiOff++)
        {
            uiCanZeroNum = 0;
            for (uint32_t uiCur = uiOff; uiCur < uiOff + uiW; uiCur++)
            {
                if (uiCur >= 255)
                {
                    break;
                }
                uiCanZeroNum += mapImgColor[uiCur];
            }
            if (uiCanZeroNum >= uiAllZeroPoint) {

                uiFrom = uiOff;
                uiTo = uiOff + uiW;
                uiW = 256;
                break;
            }
        }
    }

    cout << "Find point : From " << uiFrom << " to " << uiTo << " uiCanZeroNum = " << uiCanZeroNum << endl;


    uint32_t uiCountNumZero = 0;
    for (uint32_t i = 0; i < dstImg.rows; i++)
    {
        for (uint32_t j = 0; j < dstImg.cols; j++)
        {
            uint8_t ubVal = dstImg.at<uint8_t>(i, j);
            if (ubVal >= uiFrom && ubVal < uiTo)
            {
                dstImg.at<uint8_t>(i, j) = 0;
                uiCountNumZero++;
            }
        }
    }
    cout << "uiCountNumZero = " << uiCountNumZero << endl;
    imwrite(desPath + ".X.jpg", dstImg);

    //Output the matrix
    ofstream of;
    of.open(matPath, ios::out);
    cout << "dstImg.rows = " << dstImg.rows << endl;
    cout << "dstImg.cols = " << dstImg.cols << endl;
    for (uint32_t i = 0; i < dstImg.rows; i++)
    {
        for (uint32_t j = 0; j < dstImg.cols; j++)
        {
            uint8_t ubVal = dstImg.at<uint8_t>(i, j);
            double dTmp = (double)ubVal;
            dTmp = dTmp / 255;
            if (dTmp > 0.5)
            {
                dTmp *= 2;
            }
            else if(dTmp < 0.5)
            of << dTmp << "\t";
        }
    }

    of.close();


}



