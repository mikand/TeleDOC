#ifndef __TELEDOC_H__
#define __TELEDOC_H__

#include <VideoTransportClient.hpp>
#ifdef WIN32
    #include <WinShm.hpp>
#else
    #include <PosixShm.hpp>
#endif
#include <QImage>

// #include <opencv/cv.h>
// #include <opencv/highgui.h>

#include <opencv2/opencv.hpp>


#define DEFAULT_TOLERANCE 5

class TeledocRenderer {
 public :
  enum TRACK_POSITION {
    CENTER,
    NORTH,
    SOUTH,
    WEST,
    EAST,
    ERROR
  };

  TeledocRenderer(int centerSize, int color, int tolerance=DEFAULT_TOLERANCE);

  bool newFrameAvailable();
  int getKey();
  TRACK_POSITION getCurrentPosition();
  void setColor(int color);

  IplImage* last_frame;

 private:
  IplImage* getFrameImage();
  TRACK_POSITION getPosition(IplImage* frame);
  IplImage* qImageToIplImage(const QImage * qImage);
  IplImage* getThresholdedImage(IplImage* img);
  TRACK_POSITION getPosition(int x, int y);
  void computeEdges();
  void updateDebug(IplImage* frame, IplImage* thresholded, int x, int y);

#ifdef WIN32
  VideoTransportClient <WinShm> ipc;
#else
  VideoTransportClient <PosixShm> ipc;
#endif

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
