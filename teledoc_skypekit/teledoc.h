#ifndef __TELEDOC_H__
#define __TELEDOC_H__

#include <VideoTransportClient.hpp>
#ifdef WIN32
#include <WinShm.hpp>
#else
#include <PosixShm.hpp>
#endif
#include <QImage>

#include "tracker.h"


class TeledocRenderer {
 public :

  TeledocRenderer(int centerSize, int color, int tolerance=DEFAULT_TOLERANCE);

  bool newFrameAvailable();
  int getKey();
  Tracker::TRACK_POSITION getCurrentPosition();
  void setColor(int color);

  IplImage* last_frame;

 private:
  IplImage* getFrameImage();
  IplImage* qImageToIplImage(const QImage * qImage);

#ifdef WIN32
  VideoTransportClient <WinShm> ipc;
#else
  VideoTransportClient <PosixShm> ipc;
#endif

  /** The tracker instance */
  Tracker tracker;
};

#endif
