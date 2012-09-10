#include "teledoc.h"
#include <boost/python.hpp>

using namespace boost::python;


#define SID_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
  ( (unsigned long)(unsigned char)(ch0)         | ( (unsigned long)(unsigned char)(ch1) << 8 ) | \
    ( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )



TeledocRenderer::TeledocRenderer() {
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

  return qImageToIplImage(img);
}

int TeledocRenderer::getKey() {
  return ipc.key();
}


IplImage* TeledocRenderer::qImageToIplImage(const QImage * qImage)
{
  int width = qImage->width();
  int height = qImage->height();
  CvSize Size;
  Size.height = height;
  Size.width = width;
    
  IplImage *charIplImageBuffer = cvCreateImage(Size, IPL_DEPTH_8U, 1);
  char *charTemp = (char *) charIplImageBuffer->imageData;
 
  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
        {
          int index = y * width + x;
          charTemp[index] = (char) qGray(qImage->pixel(x, y));
        }
    }
  return charIplImageBuffer;
}
  

IplImage* TeledocRenderer::getThresholdedImage(IplImage* img)
{
	// Convert the image into an HSV image
	IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
	cvCvtColor(img, imgHSV, CV_BGR2HSV);

	IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);

	// Values 20,100,100 to 30,255,255 working perfect for yellow at around 6pm
	cvInRangeS(imgHSV, colorLB, colorUB, imgThreshed);

	cvReleaseImage(&imgHSV);

	return imgThreshed;
}


TeledocRenderer::TRACK_POSITION TeledocRenderer::getPosition(IplImage* frame){

  // Will hold a frame captured from the camera
  // IplImage* frame = 0;
  // frame = cvQueryFrame(capture);

  // Holds the yellow thresholded image (yellow = white, rest = black)
  IplImage* imgYellowThresh = getThresholdedImage(frame);

  // Calculate the moments to estimate the position of the ball
  CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
  cvMoments(imgYellowThresh, moments, 1);

  // The actual moment values
  double moment10 = cvGetSpatialMoment(moments, 1, 0);
  double moment01 = cvGetSpatialMoment(moments, 0, 1);
  double area = cvGetCentralMoment(moments, 0, 0);

  // Holding the last and current ball positions
  int posX = 0;
  int posY = 0;

  posX = moment10/area;
  posY = moment01/area;

  // Print it out for debugging purposes
  printf("position (%d,%d)\n", posX, posY);

  // Release the thresholded image... we need no memory leaks.. please
  cvReleaseImage(&imgYellowThresh);

  delete moments;

  return getPosition(posX, posY);
}


TeledocRenderer::TRACK_POSITION getPosition(int x, int y) {
  /* TODO [APP] : compute position */
  return TeledocRenderer::ERROR;
}


TeledocRenderer::TRACK_POSITION TeledocRenderer::getCurrentPosition() {
  IplImage* img = getFrameImage();
  TeledocRenderer::TRACK_POSITION res = getPosition(img);
  cvReleaseImage(&img);
  return res;
}

  



BOOST_PYTHON_MODULE(teledoc)
{
class_<TeledocRenderer>("TeledocRenderer")
.def("getKey", &TeledocRenderer::getKey)
.def("newFrameAvailable", &TeledocRenderer::getKey)
.def("getCurrentPosition", &TeledocRenderer::getCurrentPosition)
;
}







// static PyObject *
// teledoc_get_frame(PyObject *self, PyObject *args);


// static PyMethodDef TeledocMethods[] = {
//     {"getCurrentFrame",  teledoc_get_frame, METH_VARARGS, "Gets a matrix of RGB values"},
//     {NULL, NULL, 0, NULL}        /* Sentinel */
// };


// PyMODINIT_FUNC
// initteledoc(void)
// {
//     (void) Py_InitModule("teledoc", TeledocMethods);
// }


// static PyObject *
// teledoc_get_frame(PyObject *self, PyObject *args)
// {
//     const char *command;
//     int sts;

//     if (!PyArg_ParseTuple(args, "s", &command))
//         return NULL;
//     sts = system(command);
//     return Py_BuildValue("i", sts);
// }
