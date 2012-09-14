#ifndef __TRACKER_H__
#define __TRACKER_H__

#ifndef OPENCV_2
#include <opencv/cv.h>
#include <opencv/highgui.h>
#else
#include <opencv2/opencv.hpp>
#endif


#define DEFAULT_TOLERANCE 5

#define SID_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
  ( (unsigned long)(unsigned char)(ch0)         | ( (unsigned long)(unsigned char)(ch1) << 8 ) | \
    ( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )


class Tracker {
public :
  enum TRACK_POSITION {
    CENTER,
    NORTH,
    SOUTH,
    WEST,
    EAST,
    ERROR
  };

  Tracker(int centerSize, int color, int tolerance=DEFAULT_TOLERANCE);

  TRACK_POSITION getPosition(IplImage* frame);

  void setColor(int color);


private:
  IplImage* getThresholdedImage(IplImage* img);
  TRACK_POSITION getPosition(int x, int y);
  void computeEdges();
  void updateDebug(IplImage* frame, IplImage* thresholded, int x, int y);

  CvScalar colorLB; /* Lower-bound of the color to track */
  CvScalar colorUB; /* Upper-bound of the color to track */
  int center_size;  /* Size of the central area in percentage */

  int tolerance;

  int frame_width;  /* Size of the frames  */
  int frame_height;

  /* Edges positions */
  int north_edge;
  int south_edge;
  int east_edge;
  int west_edge;

};

#endif
