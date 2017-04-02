#include "C2Lsh.h"

#include <stdint.h>
#include <string>
#include <string.h>
#include <iostream>
#include <math.h>
#include <random>
#include <algorithm>


using namespace std;
namespace caravel {

    C2Lsh::C2Lsh()
    {
        m_parLshFunction = NULL;
    }


    C2Lsh::~C2Lsh()
    {
    }

    void C2Lsh::Init(uint32_t uiD, uint32_t uiL, double dW)
    {
        m_uiD = uiD;
        m_uiL = uiL;
        m_dW = dW;

        m_parLshFunction = new PLSHFUNC[uiL];

        for (uint32_t uiCur = 0; uiCur < uiL; uiCur++)
        {
            m_parLshFunction[uiCur] = new stLshFunction();

            PLSHFUNC pFunc = m_parLshFunction[uiCur];
            pFunc->arA = new double[uiD];

            //init Gaussian Random Matrix a
            for (uint32_t uiCurD = 0; uiCurD < uiD; uiCurD++)
            {
                pFunc->arA[uiCurD] = genGaussianRandom();
                //cout << "arA = " << pFunc->arA[uiCurD] << "   ";
            }
            //init Uniform Random b [0 - w]
            pFunc->dB = genUniformRandom(0, m_dW);
            //cout << "\ndB = " << pFunc->dB << "   \n";

        }

    }

    void C2Lsh::Compute(double *arMatrix, uint32_t *aruiRet)
    {
        //compute

        for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
        {
            PLSHFUNC pFunc = m_parLshFunction[uiCur];

            double dLsh = 0;
            for (uint32_t uiCurD = 0; uiCurD < m_uiD; uiCurD++)
            {
                dLsh += arMatrix[uiCurD] * pFunc->arA[uiCurD];
                //cout << "Value = " << arMatrix[uiCurD] * pFunc->arA[uiCurD] << endl;

            }

            dLsh += pFunc->dB;
            dLsh /= m_dW;

            //cout << "LSH = " << dLsh << endl;

            aruiRet[uiCur] = (uint32_t)ceil(dLsh);
        }

    }


    double C2Lsh::genGaussianRandom()
    {
        static random_device rd;
        static mt19937 mt(rd());
        static normal_distribution<float> gr(0.0, 1.0);
        return gr(mt);
    }

    uint32_t C2Lsh::genUint32Random(uint32_t uiRangeStart, uint32_t uiRangeEnd)
    {
        static random_device rd;
        static mt19937 mt(rd());
        uniform_int_distribution<uint32_t> gr(uiRangeStart, uiRangeEnd);
        return gr(mt);
    }

    double C2Lsh::genUniformRandom(double dRangeStart, double dRangeEnd)
    {
        static random_device rd;
        static mt19937 mt(rd());
        uniform_real_distribution<double> gr(dRangeStart, dRangeEnd);
        return gr(mt);
    }


    double C2Lsh::ComputeL2(double *ardX, double *ardY, uint32_t uiD)
    {
        double dRet = 0;
        for (uint32_t uiCur = 0; uiCur < uiD; uiCur++)
        {
            dRet += (ardY[uiCur] - ardX[uiCur]) * (ardY[uiCur] - ardX[uiCur]);
        }
        return sqrt(dRet);
    }


}

