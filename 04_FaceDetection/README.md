# Face Detection with Qt and OpenCV: Chapter 04 Reflections

**Author**: Tony Fu  
**Date**: August 9, 2023  
**Device**: MacBook Pro 16 inch, Late 2021 (M1 Pro)  

**Reference**: Chapter 4 of [*Qt 5 and OpenCV 4 Computer Vision Projects*](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang

## Results:



## Core Concepts

### 1. Issues Encountered & Solutions:

1. `resetMatrix` is deprecated:
  Solution: Replace `imageView->resetTransform();` with `imageView->resetTransform();` This deprecation warning has actually showed up in the previous chapters, but I ignored it at the time.

2. The app crashed after closing, giving the error 
```
Thread 6 Crashed:: CaptureThread
0   libopencv_core.4.8.0.dylib    	       0x10493b4e8 cv::parallel_for_(cv::Range const&, cv::ParallelLoopBody const&, double) + 516
1   libopencv_core.4.8.0.dylib    	       0x10493b4dc cv::parallel_for_(cv::Range const&, cv::ParallelLoopBody const&, double) + 504
2   libopencv_imgproc.4.8.0.dylib 	       0x104e5db10 cv::hal::cvtBGRtoBGR(unsigned char const*, unsigned long, unsigned char*, unsigned long, int, int, int, int, int, bool) + 196
3   libopencv_imgproc.4.8.0.dylib 	       0x104e60c6c cv::cvtColorBGR2BGR(cv::_InputArray const&, cv::_OutputArray const&, int, bool) + 432
4   libopencv_imgproc.4.8.0.dylib 	       0x104e4796c cv::cvtColor(cv::_InputArray const&, cv::_OutputArray const&, int, int) + 2212
5   04_FaceDetection              	       0x1044b94d4 CaptureThread::run() + 216
6   QtCore                        	       0x106849e44 0x106828000 + 138820
7   libsystem_pthread.dylib       	       0x1a2c2a06c _pthread_start + 148
8   libsystem_pthread.dylib       	       0x1a2c24e2c thread_start + 8
```

This suggests that our CaptureThread::run() may not be doing the right cleanup. 

Solution: None so far.

3. 



