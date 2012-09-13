#include "tracker.h"

#include <iostream>

using namespace std;

#define GRAPHICAL_DEBUG


static void mouse_callback(int event, int x, int y, int flags, void* param);

Tracker::Tracker(int centerSize, int color, int tolerance)
{
  frame_width = 0;
  frame_height = 0;

  center_size = centerSize;

  this->tolerance = tolerance;

  colorLB = cvScalar(color-tolerance, 100, 100);
  colorUB = cvScalar(color+tolerance, 255, 255);
}


IplImage* Tracker::getThresholdedImage(IplImage* img)
{
  // Convert the image into an HSV image
  IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
  cvCvtColor(img, imgHSV, CV_BGR2HSV);

  IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);
  
  /* [APP] Smoothing can occur here */
  // Values 20,100,100 to 30,255,255 working perfect for yellow at around 6pm
  cvInRangeS(imgHSV, colorLB, colorUB, imgThreshed);

  /* [APP] Smoothing can occur here */

  cvReleaseImage(&imgHSV);

  return imgThreshed;
}


Tracker::TRACK_POSITION Tracker::getPosition(IplImage* frame)
{
  int fw = cvGetSize(frame).width;
  int fh = cvGetSize(frame).height;

  if (fw != frame_width || fh != frame_height) {
    /* First initialization */
    cout << "Initializing constraints..." << endl;

    frame_width = fw;
    frame_height = fh;

    cout << "  frame_width = " << frame_width << endl;
    cout << "  frame_height = " << frame_height << endl;

    computeEdges();
  }


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

  /* [APP] Area can be used to avoid processing noise */

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
Tracker::TRACK_POSITION Tracker::getPosition(int x, int y) {
  
  if ( x < 0 || y < 0 ) 
    return Tracker::ERROR;
  else if ( x > frame_width || y > frame_height )
    return Tracker::ERROR;
  else if ( x > east_edge)
    return Tracker::EAST;
  else if ( x < west_edge)
    return Tracker::WEST;
  else if ( y < north_edge)
    return Tracker::NORTH;
  else if ( y > south_edge)
    return Tracker::SOUTH;
  else
    return Tracker::CENTER;
}

/* Computes the edges location by making some calculation
   over the image size.
 */
void Tracker::computeEdges(){
  

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


void Tracker::updateDebug(IplImage* frame, IplImage* thresholded, int x, int y){
  cvNamedWindow("video");
  cvNamedWindow("thresh");
  
  /* Draw Edges */
  /* Maybe move globally, during ComputeEdges? */
  IplImage* imgEdges = cvCreateImage(cvGetSize(frame), 8, 3);

  cvMerge(thresholded, thresholded, thresholded, NULL, imgEdges);

  cvLine(imgEdges, 
	 cvPoint(0, north_edge), 
	 cvPoint(frame_width, north_edge), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(0, south_edge), 
	 cvPoint(frame_width, south_edge), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(east_edge, 0), 
	 cvPoint(east_edge, frame_height), 
	 cvScalar(0,255,255), 5);

  cvLine(imgEdges, 
	 cvPoint(west_edge, 0), 
	 cvPoint(west_edge, frame_height), 
	 cvScalar(0,255,255), 5);

  cvCircle(imgEdges, cvPoint(x,y), 2, cvScalar(0, 255, 255));
  /* Maybe we should work on a copy of thresholded ? */
  //cvAdd(imgEdges, thresholded, imgEdges);

  cvShowImage("thresh", imgEdges);
  cvShowImage("video", frame);

  cvSetMouseCallback("video", mouse_callback, (void*) this);

  cvWaitKey(10);

  cvReleaseImage(&imgEdges);
} 

void Tracker::setColor(int color) {
  colorLB = cvScalar(color - this->tolerance, 50, 50);
  colorUB = cvScalar(color + this->tolerance, 255, 255);
}


static void mouse_callback(int event, int x, int y, int flags, void* param) {
  Tracker* tr = (Tracker*) param;
  // IplImage* img = (IplImage*) tr->last_frame;

  // if (event == CV_EVENT_LBUTTONDOWN) {
  //   //cout << "X=" << x << ", Y=" << y << endl;
  //   //    CvScalar s = cvGet2D(image, x, y);
  //   tr->setColor(((uchar *)(img->imageData + x*img->widthStep))[y*img->nChannels +0]);
  // } 
  
}
