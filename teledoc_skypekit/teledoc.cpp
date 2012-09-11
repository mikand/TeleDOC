#include "teledoc.h"
#include <boost/python.hpp>

using namespace boost::python;


#define SID_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
  ( (unsigned long)(unsigned char)(ch0)         | ( (unsigned long)(unsigned char)(ch1) << 8 ) | \
    ( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )

#define GRAPHICAL_DEBUG 1


TeledocRenderer::TeledocRenderer(int centerSize, int color, int tolerance) {
  // Here we will inform the runtime that we are capable of drawing
  // RGB32 and RGB24 bitmaps. The runtime will pick the most suitable
  // of those. Format of the frames we receive back will be included in the
  // bufferstruct's colorspace and bitcount fields

  unsigned int list[2];
  list[0] = SID_MAKEFOURCC('B','I','3','2');
  list[1] = SID_MAKEFOURCC('B','I','2','4');
  ipc.setPreferences(2, list);

  frame_width = 0;
  frame_height = 0;
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

#ifdef GRAPHICAL_DEBUG
  updateDebug(frame, imgYellowThresh, posX, posY);
#else
  printf("position (%d,%d)\n", posX, posY);
#endif

  // Release the thresholded image... we need no memory leaks.. please
  cvReleaseImage(&imgYellowThresh);

  delete moments;

  return getPosition(posX, posY);
}

/* Compute the position by comparing the values (x,y) with 
   the edges of the areas. Returns ERROR if the points are 
   outside of the picture.
*/
TeledocRenderer::TRACK_POSITION TeledocRenderer::getPosition(int x, int y) {
  
  if ( x < 0 || y < 0 ) 
    return TeledocRenderer::ERROR;
  else if ( x > frame_width || y > frame_height )
    return TeledocRenderer::ERROR;
  else if ( x > east_edge)
    return TeledocRenderer::EAST;
  else if ( x < west_edge)
    return TeledocRenderer::WEST;
  else if ( y < north_edge)
    return TeledocRenderer::NORTH;
  else if ( y > south_edge)
    return TeledocRenderer::SOUTH;
  else
    return TeledocRenderer::ERROR;
}

/* Computes the edges location by making some calculation
   over the image size.
 */
void TeledocRenderer::computeEdges(){
  

  int central_height = (center_size * frame_height)/100;
  int central_width = (center_size * frame_width)/100;

  north_edge = (frame_height - central_height)/2;
  south_edge = frame_height - north_edge;

  west_edge = (frame_width - central_width) /2;
  east_edge = frame_width - west_edge;

  assert(north_edge >= 0 );
  assert(south_edge >= 0 );
  assert(east_edge >= 0 );
  assert(west_edge >= 0 );
}


TeledocRenderer::TRACK_POSITION TeledocRenderer::getCurrentPosition() {
  IplImage* img = getFrameImage();

  if (0 == frame_width && 0 == frame_height) {
    /* First initialization */
    
    frame_width = cvGetSize(img).width;
    frame_height = cvGetSize(img).height;

    computeEdges();
  }

  TeledocRenderer::TRACK_POSITION res = getPosition(img);
  cvReleaseImage(&img);
  return res;
}

void TeledocRenderer::updateDebug(IplImage* frame, IplImage* thresholded, int x, int y){
  cvNamedWindow("video");
  cvNamedWindow("thresh");

  /* Draw Edges */
  /* Maybe move globally, during ComputeEdges? */
  IplImage* imgEdges = cvCreateImage(cvGetSize(frame), 8, 3);
  cvLine(imgEdges, 
	 cvPoint(0, north_edge), 
	 cvPoint(frame_width, north_edge), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(0, south_edge), 
	 cvPoint(frame_width, south_edge), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(0, 0), 
	 cvPoint(east_edge, frame_height), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(0, 0), 
	 cvPoint(west_edge, frame_height), 
	 cvScalar(0,255,255), 5);

  cvCircle(imgEdges, cvPoint(x,y), 2, cvScalar(0, 255, 255));
  /* Maybe we should work on a copy of thresholded ? */
  cvAdd(thresholded, imgEdges, thresholded);
  cvShowImage("tresh", thresholded);
  cvShowImage("video", frame);

  cvReleaseImage(&imgEdges);
  
}  



BOOST_PYTHON_MODULE(teledoc)
{
  class_<TeledocRenderer>("TeledocRenderer", init<int,int,int>())
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
