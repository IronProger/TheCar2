//
// Created by rege on 21.03.19.
//

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <plog/Log.h>
#include <experimental/filesystem>
#include <opencv2/imgcodecs.hpp>
#include "CNN.hpp"
#include "Detect.hpp"

using namespace std;
using namespace cv;
using namespace experimental::filesystem;

void CNN::runKernel (const Mat1f & src, Mat1f & dst, const Mat1f & kernel)
{
    // assertion and verification block
    assert(kernel.rows == kernel.cols);
    assert(kernel.type() == CV_32FC1);
    assert(src.type() == CV_32FC1);
    assert(src.cols > 0 && src.rows > 0);
    int kernelSize;
    if (kernel.rows == 3) kernelSize = 3;
    else if (kernel.rows == 5) kernelSize = 5;
    else
    {
        LOGF << "You gave me illegal kernel is not sized for 3x3 or 5x5. Then just I'll terminate.";
        exit(15);
    }

    // get dot multiplication of two matrices and get sum of all elements of result matrix
    auto mulAndSumKernels = [] (const Mat1f & k0, const Mat1f & k1) -> float
    {
        Mat1f m = k0.mul(k1);

        float sum = 0;
        for (float f : Mat1f(m))
        {
            if (f == NAN || f == INFINITY)
            {
                LOGW << "There is NAN or INFINITY";
                f = 0;
            }
            sum += f;
        }
        return sum;
    };

    Mat1f result(src.size());

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            Mat1f localKernel;
            getKernelByPoint(x, y, src, localKernel, kernelSize);
            result.at<float>(y, x) = mulAndSumKernels(localKernel, kernel);
        }
    }
    dst = result;
}

void CNN::getKernelByPoint (int x, int y, const Mat1f & src, Mat1f & outputKernel, int kernelSize)
{
    assert(kernelSize == 3 || kernelSize == 5);
    cv::Mat1f k(kernelSize, kernelSize);
    for (int ty = 0; ty < kernelSize; ty++)
    {
        for (int tx = 0; tx < kernelSize; tx++)
        {
            k.at<float>(ty, tx) = src.at<float>(
                    constrain(y - (kernelSize - 1) / 2 + ty, 0, src.rows - 1),
                    constrain(x - (kernelSize - 1) / 2 + tx, 0, src.cols - 1)
            );
        }
    }
    outputKernel = k;
}

void CNN::maxPooling (Mat1f & src, Mat1f & dst, int kernelWidth, int kernelHeight)
{
    if (kernelHeight > src.rows || kernelWidth > src.cols)
    {
        LOGF << "kernel is too small (kernelSize > src.rows || kernelSize > src.cols)";
        exit(16);
    }

    if (src.rows % kernelHeight != 0 || src.cols % kernelWidth != 0)
    {
        LOGF << "(src.rows%kernelHeight!=0 || src.cols%kernelWidth!=0) fails";
        exit(17);
    }

    Mat1f result(src.rows / kernelHeight, src.cols / kernelWidth);

    // returns maximum element of given matrix
    auto maxOfMat1f = [] (const Mat1f & m) -> float
    {
        float maximum = 0;
        for (float & f : Mat_<float>(m)) if (f > maximum) maximum = f;
        return maximum;
    };

    for (int y = 0; y < src.rows / kernelHeight; y++)
    {
        for (int x = 0; x < src.cols / kernelWidth; x++)
        {
            Mat_<float> region = Mat_<float>(
                    src,
                    Range(y * kernelHeight, y * kernelHeight + kernelHeight),
                    Range(x * kernelWidth, x * kernelWidth + kernelWidth)
            );
            result.at<float>(y, x) = maxOfMat1f(region);
        }
    }

    dst = result;
}

void CNN::runReLU (Mat1f & mat)
{
    for (float & f : mat)
    {
        f = f > 0 ? f : 0.0f;
    }
};

void CNN::process (const Mat1b & src, Mat1f & dst)
{
    assert(src.cols != 0 && src.rows != 0);
    assert(src.cols == IMAGE_WIDTH && src.rows == IMAGE_HEIGHT);

    // debug
    static int a = 0;
    imwrite("/tmp/asd/" + to_string(a++) + ".png", src);

    vector<Mat_<float>> kernels{
            // top side of white air
            (Mat_<float>(5, 5) << -0.1f, -0.1f, -0.1f, -0.1f, -0.1f,
                    -0.1f, -0.1f, -0.1f, -0.1f, -0.1f,
                    0.066f, 0.066f, 0.066f, 0.066f, 0.066f,
                    0.066f, 0.066f, 0.066f, 0.066f, 0.066f,
                    0.066f, 0.066f, 0.066f, 0.066f, 0.066f),
            // bottom side of white air
            (Mat_<float>(5, 5) << 0.066f, 0.066f, 0.066f, 0.066f, 0.066f,
                    0.066f, 0.066f, 0.066f, 0.066f, 0.066f,
                    0.066f, 0.066f, 0.066f, 0.066f, 0.066f,
                    -0.1f, -0.1f, -0.1f, -0.1f, -0.1f,
                    -0.1f, -0.1f, -0.1f, -0.1f, -0.1f),
            // left side of white air
            (Mat_<float>(5, 5) << -0.1f, -0.1f, 0.066f, 0.066f, 0.066f,
                    -0.1f, -0.1f, 0.066f, 0.066f, 0.066f,
                    -0.1f, -0.1f, 0.066f, 0.066f, 0.066f,
                    -0.1f, -0.1f, 0.066f, 0.066f, 0.066f,
                    -0.1f, -0.1f, 0.066f, 0.066f, 0.066f),
            // right side of white air
            (Mat_<float>(3, 3) << 0.066f, 0.066f, 0.066f, -0.1f, -0.1f,
                    0.066f, 0.066f, 0.066f, -0.1f, -0.1f,
                    0.066f, 0.066f, 0.066f, -0.1f, -0.1f,
                    0.066f, 0.066f, 0.066f, -0.1f, -0.1f,
                    0.066f, 0.066f, 0.066f, -0.1f, -0.1f)
    };

    vector<Mat1f> matrices;

    int n = 0; //debug
    for (const Mat_<float> & k : kernels)
    {
        Mat1f m;

        // convolution
        runKernel(src, m, k);

        // max pulling
        maxPooling(m, m, 3, 3);

        matrices.emplace_back(m);
    }

    // merge all matrices in one
    for (int i = 1; i < matrices.size(); i++)
    {
        matrices.at(0).push_back(matrices.at(i));
    }

    dst = matrices.at(0);

    // apply ReLU
    runReLU(dst);
}