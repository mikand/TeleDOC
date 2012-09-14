#include "tracker.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Usage " << argv[0] << " <color>" << endl;
    return 0;
  }

  int color = atoi(argv[1]);
  Tracker t(50, color);

  CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );
  if ( !capture ) {
    fprintf( stderr, "ERROR: capture is NULL \n" );
    getchar();
    return -1;
  }

  // Show the image captured from the camera in the window and repeat
  while ( 1 ) {
    // Get one frame
    IplImage* frame = cvQueryFrame( capture );
    if ( !frame ) {
      fprintf( stderr, "ERROR: frame is null...\n" );
      getchar();
      break;
    }

    switch(t.getPosition(frame)) {
    case Tracker::CENTER:
      cout << "CENTER" << endl;
      break;

    case Tracker::EAST:
      cout << "EAST" << endl;
      break;

    case Tracker::WEST:
      cout << "WEST" << endl;
      break;

    case Tracker::NORTH:
      cout << "NORTH" << endl;
      break;

    case Tracker::SOUTH:
      cout << "SOUTH" << endl;
      break;

    default:
      cout << "ERROR" << endl;
    }

    // Do not release the frame!
    //If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
    //remove higher bits using AND operator
    if ( (cvWaitKey(10) & 255) == 27 ) break;
  }
  // Release the capture device housekeeping
  cvReleaseCapture( &capture );
  return 0;
}
