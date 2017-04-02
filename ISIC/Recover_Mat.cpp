#include <iostream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>

#include <KL1pInclude.h>

using namespace std;
using namespace kl1p;

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

    uint32_t uiMatrixSize;
    iff >> uiMatrixSize;

    arma::Col<klab::DoubleReal> y;
    y.set_size(uiMatrixSize);
    y.fill(0.0);

    for (uint32_t uiCur = 0; uiCur < uiMatrixSize; uiCur++)
    {
        double dVal;
        iff >> dVal;
        y[uiCur] = dVal;
        if (uiCur < 30)
        {
            cout << y[uiCur] << " ";
        }
    }

    //Init params

    uint32_t uiSize = DEF_IMG_W * DEF_IMG_H;

    klab::UInt32 n = uiSize;		    	// Size of the original signal x0.
    klab::DoubleReal alpha = 0.5;			// Ratio of the cs-measurements.
    klab::DoubleReal rho = 0.25;				// Ratio of the sparsity of the signal x0.
    klab::UInt32 m = klab::UInt32(alpha*n);	// Number of cs-measurements.
    klab::UInt32 k = klab::UInt32(rho*n);	// Sparsity of the signal x0 (number of non-zero elements).
    klab::UInt64 seed = 1;					// Seed used for random number generation (0 if regenerate random numbers on each launch).

    if (seed > 0)
        klab::KRandom::Instance().setSeed(seed);

    // Display signal informations.
    std::cout << "==============================" << std::endl;
    std::cout << "N=" << n << " (signal size)" << std::endl;
    std::cout << "M=" << m << "=" << std::setprecision(5) << (alpha*100.0) << "% (number of measurements)" << std::endl;
    std::cout << "K=" << k << "=" << std::setprecision(5) << (rho*100.0) << "% (signal sparsity)" << std::endl;
    std::cout << "Random Seed=" << klab::KRandom::Instance().seed() << std::endl;
    std::cout << "==============================" << std::endl;

    // Create random gaussian i.i.d matrix A of size (m,n).
    klab::TSmartPointer<kl1p::TOperator<klab::DoubleReal> > A = new kl1p::TNormalRandomMatrixOperator<klab::DoubleReal>(m, n, 0.0, 1.0);
    A = new kl1p::TScalingOperator<klab::DoubleReal>(A, 1.0 / klab::Sqrt(klab::DoubleReal(m)));	// Pseudo-normalization of the matrix (required for AMP and EMBP solvers).		


    // Compute CoSaMP.
    std::cout << "------------------------------" << std::endl;
    std::cout << "[CoSaMP] Start." << std::endl;

    klab::DoubleReal tolerance = 1e-3;	// Tolerance of the solution.
    arma::Col<klab::DoubleReal> x; //store the result

    kl1p::TCoSaMPSolver<klab::DoubleReal> cosamp(tolerance);
    cosamp.solve(y, A, k, x);

    cout << "Finish Reconstruction x.n_rows = " << x.n_rows << endl;

    ofstream off;
    off.open(desPath, ios::out);
    for (uint32_t uiCur = 0; uiCur < x.n_rows; uiCur++)
    {
        off << x[uiCur] << "\t";
    }

    off.close();

}



