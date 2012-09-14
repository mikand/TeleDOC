#include "teledoc.h"
#include <boost/python.hpp>

#include <iostream>

using namespace std;
using namespace boost::python;


TeledocRenderer::TeledocRenderer(int centerSize, int color, int tolerance) :
  tracker(centerSize, color, tolerance)
{
  // Here we will inform the runtime that we are capable of drawing
  // RGB32 and RGB24 bitmaps. The runtime will pick the most suitable
  // of those. Format of the frames we receive back will be included in the
  // bufferstruct's colorspace and bitcount fields

  unsigned int list[2];
  list[0] = SID_MAKEFOURCC('B','I','3','2');
  list[1] = SID_MAKEFOURCC('B','I','2','4');
  ipc.setPreferences(2, list);
}


bool TeledocRenderer::newFrameAvailable() {
  VideoTransportBase::bufferstruct *buffer = ipc.getNewFrame();
  return (buffer != NULL);
}


IplImage* TeledocRenderer::getFrameImage() {
  VideoTransportBase::bufferstruct *buffer = ipc.getFrame();
  if (!buffer) {
    return NULL;
  }

  // Now we need to check the actual bitmap format of the frame
  QImage::Format format;

  if (buffer->fourcc == SID_MAKEFOURCC('B','I','3','2')) {
    format = QImage::Format_RGB32;
  }
  else if (buffer->fourcc == SID_MAKEFOURCC('B','I','2','4')) {
    format = QImage::Format_RGB888;
  }
  else if (buffer->fourcc == 0) {
    // FOURCC == 0 is also a valid value, in which case the actual format can be
    // either RGB32, RGB24 or RGB 16. Between those we can choose based on buffer's bitcount

    if (buffer->bitsperpixel == 32) {
      format = QImage::Format_RGB32;
    }
    else if (buffer->bitsperpixel == 24) {
      format = QImage::Format_RGB888;
    }
    else if (buffer->bitsperpixel == 16) {
      format = QImage::Format_RGB16;
    }
    else {
      return NULL;
    }
  }
  else {
    return NULL;
  }

  // Creating image
  QImage* img = new QImage((const uchar *)ipc.bufferData(buffer),
                           buffer->width,
                           buffer->height,
                           format);

  IplImage* res = qImageToIplImage(img);

  delete img;

  return res;
}

int TeledocRenderer::getKey() {
  return ipc.key();
}


IplImage* TeledocRenderer::qImageToIplImage(const QImage * qimg)
{
  IplImage *imgHeader = cvCreateImageHeader( cvSize(qimg->width(), qimg->height()), IPL_DEPTH_8U, 4);
  imgHeader->imageData = (char*) qimg->bits();

  uchar* newdata = (uchar*) malloc(sizeof(uchar) * qimg->byteCount());
  memcpy(newdata, qimg->bits(), qimg->byteCount());
  imgHeader->imageData = (char*) newdata;

  //cvClo
  return imgHeader;
}
  

Tracker::TRACK_POSITION TeledocRenderer::getCurrentPosition() {
  IplImage* img = getFrameImage();

  if (NULL == img) {
    cout << "Retrived a NULL image..." << endl;
    return Tracker::ERROR;
  }

  Tracker::TRACK_POSITION res = tracker.getPosition(img);
  cvReleaseImage(&img);
  return res;
}

 
void TeledocRenderer::setColor(int color) {
  tracker.setColor(color);
}


BOOST_PYTHON_MODULE(teledoc)
{
  class_<TeledocRenderer>("TeledocRenderer", init<int,int,int>())
.def("getKey", &TeledocRenderer::getKey)
.def("newFrameAvailable", &TeledocRenderer::getKey)
.def("getCurrentPosition", &TeledocRenderer::getCurrentPosition)
;

  enum_<Tracker::TRACK_POSITION >("TRACK_POSITION")
    .value("CENTER", Tracker::CENTER)
    .value("NORTH", Tracker::NORTH)
    .value("SOUTH", Tracker::SOUTH)
    .value("WEST", Tracker::WEST)
    .value("EAST", Tracker::EAST)
    .value("ERROR", Tracker::ERROR)
    ;
}
