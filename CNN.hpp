//
// Created by rege on 21.03.19.
//

#ifndef THECAR_CNN_HPP
#define THECAR_CNN_HPP

/**
 * Note:
 * This class doesn't claim to be a real convolution neural network. However it was conceived thus at begin.
 * Now it contain a small functional basis needs to build a CNN. Function "process" performs a very simple
 * single layer CNN with manual set parameters. It cannot be trained, which makes each neural network a neural network.
 */

class CNN
{
    /**
     * It's a simple convolutional layer realisation.
     * Run kernel for one-channel image and write result to dst, kernel can be sized for 3x3 or 5x5.
     * @note thread-safe
     */
    void runKernel (const cv::Mat1f & src, cv::Mat1f & dst, const cv::Mat1f & kernel);

    /**
     * get kernel based by specified coordinates on the source image
     * @note thread-safe
     */
    void getKernelByPoint (int x, int y, const cv::Mat1f & src, cv::Mat1f & outputKernel, int kernelSize);

    /**
     * @note thread-safe
     */
    void maxPooling (cv::Mat1f & src, cv::Mat1f & dst, int kernelWidth, int kernelHeight);

    /**
     * Simple ReLU (p > 0 ? p : 0)
     * @param mat
     */
    void runReLU (cv::Mat1f & mat);

public:

    /**
     * Constrains a number to be within a range.
     * @note thread-safe
     */
    template<typename number>
    inline number constrain (number value, number min, number max)
    {
        if (value > max) return max;
        if (value < min) return min;
        return value;
    }

    /**
     * The uniting function. Takes a prepared image as source, and writes to destination
     * with applying convolution, pooling, uniting to one matrix and applying ReLU.
     * @param src
     * @param dst
     */
    void process (const cv::Mat1b & src, cv::Mat1f & dst);
};

#endif //THECAR_CNN_HPP
