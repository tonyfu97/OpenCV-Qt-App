#include "cartoon_plugin.h"

QString CartoonPlugin::name()
{
    return "Cartoon";
}

void CartoonPlugin::edit(const cv::Mat &input, cv::Mat &output)
{
    int num_down = 2;
    int num_bilateral = 7;

    cv::Mat copy1, copy2;
    cv::Mat image_gray, image_edge;

    // 1. Downsample image to reduce size and noise
    copy1 = input.clone();
    for (int i = 0; i < num_down; i++)
    {
        cv::pyrDown(copy1, copy2);
        copy1 = copy2.clone();
    }

    // 2. Apply bilateral filter to smoothen colors while keeping edges sharp
    for (int i = 0; i < num_bilateral; i++)
    {
        cv::bilateralFilter(copy1, copy2, 9, 9, 7);
        copy1 = copy2.clone();
    }

    // 3. Upscale image back to original size
    for (int i = 0; i < num_down; i++)
    {
        cv::pyrUp(copy1, copy2);
        copy1 = copy2.clone();
    }

    // 4. Ensure image dimensions match the original (after upscaling)
    if (input.cols != copy1.cols || input.rows != copy1.rows)
    {
        cv::Rect rect(0, 0, input.cols, input.rows);
        copy1(rect).copyTo(copy2);
        copy1 = copy2;
    }

    // 5. Convert image to grayscale and detect edges
    cv::cvtColor(input, image_gray, cv::COLOR_RGB2GRAY);
    cv::medianBlur(image_gray, image_gray, 5);
    cv::adaptiveThreshold(image_gray, image_gray, 255,
                          cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 9, 2);
    cv::cvtColor(image_gray, image_edge, cv::COLOR_GRAY2RGB);

    // 6. Combine smoothened image with edges to produce cartoon effect
    output = copy1 & image_edge;

    /*
    cv::GaussianBlur(image_edge, image_edge, cv::Size(5, 5), 0);
    cv::Mat mask(input.rows, input.cols, CV_8UC3, cv::Scalar(90, 90, 90));
    mask = mask & (~image_edge);
    output = (copy1 & image_edge) | mask;
    */
}
