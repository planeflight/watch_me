#include <spdlog/spdlog.h>

#include <opencv2/opencv.hpp>

int main() {
    // open camera
    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        spdlog::error("Could not open camera.");
        return -1;
    }
    // cv::Mat img = cv::imread("./best.png", cv::IMREAD_COLOR);
    cv::Mat frame_gpu, processed;
    cv::Mat frame_cpu;
    while (true) {
        capture >> frame_cpu;
        if (frame_cpu.empty()) {
            spdlog::warn("Empty CPU frame.");
            break;
        }
        // upload to GPU
        frame_cpu.copyTo(frame_gpu);
        cv::cvtColor(frame_gpu, processed, cv::COLOR_BGR2GRAY);
        cv::imshow("Display", processed);

        if ((cv::waitKey(1) & 0xFF) == 27) // ESC to quit
            break;
    }
    return 0;
}
