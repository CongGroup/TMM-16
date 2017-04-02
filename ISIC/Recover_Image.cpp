#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#include "ISIC.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "Please use : " << argv[0] << " InputImageMatrix OutputImagePath" << endl;
        return -1;
    }

    //1024 X 768

    string srcPath(argv[1]);
    string desPath(argv[2]);

    ifstream iff;
    iff.open(srcPath, ios::in);

    cout << "CV_8UC1  = " << CV_8UC1 << endl;

    Mat dstImg(DEF_IMG_H, DEF_IMG_W, CV_8UC1);

    for (uint32_t i = 0; i < dstImg.rows; i++)
    {
        for (uint32_t j = 0; j < dstImg.cols; j++)
        {
            double dTmp;
            uint8_t ubVal;
            iff >> dTmp;
            ubVal = (uint8_t)(dTmp * 255);
            cout << (uint32_t)ubVal << " ";
            dstImg.at<uint8_t>(i, j) = ubVal;
        }
    }
    iff.close();

    imwrite(desPath, dstImg);

}



