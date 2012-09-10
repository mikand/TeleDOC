#include <Python.h>

#include <VideoTransportClient.hpp>
#ifdef WIN32
    #include <WinShm.hpp>
#else
    #include <PosixShm.hpp>
#endif
#include <QImage>


#define SID_MAKEFOURCC(ch0, ch1, ch2, ch3) \
        ( (unsigned long)(unsigned char)(ch0)         | ( (unsigned long)(unsigned char)(ch1) << 8 ) | \
        ( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )


class RGB {
public:
  RGB() {
    red = 0;
    green = 0;
    blue = 0;
  }

  RGB(char red, char green, char blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
  }

  char getRed() {
    return red;
  }

  void setRed(char val) {
    red = val;
  }

  char getGreen() {
    return green;
  }

  void setGreen(char val) {
    green = val;
  }

  char getBlue() {
    return blue;
  }

  void setBlue(char val) {
    blue = val;
  }
  
private:
  char red;
  char green;
  char blue;
};


class TeledocImage {
public :
  TeledocImage(QImage* img) {
    this->img = img;
  }
    
  ~TeledocImage() {
    delete img;
  }

private:
  QImage* img;
};


class TeledocRenderer {
  public :
  TeledocRenderer() {
    // Here we will inform the runtime that we are capable of drawing
    // RGB32 and RGB24 bitmaps. The runtime will pick the most suitable
    // of those. Format of the frames we receive back will be included in the
    // bufferstruct's colorspace and bitcount fields

    unsigned int list[2];
    list[0] = SID_MAKEFOURCC('B','I','3','2');
    list[1] = SID_MAKEFOURCC('B','I','2','4');
    ipc.setPreferences(2, list);
  }

  bool newFrameAvailable() {
    VideoTransportBase::bufferstruct *buffer = ipc.getNewFrame();
    return (buffer != NULL);
  }

  TeledocImage* getFrameImage() {
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
     return new TeledocImage(img);
  }

  int getKey() { return ipc.key(); }

  private:
    #ifdef WIN32
        VideoTransportClient <WinShm> ipc;
    #else
        VideoTransportClient <PosixShm> ipc;
    #endif
};




#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(teledoc)
{
    class_<TeledocRenderer>("TeledocRenderer")
        .def("getFrameImage", &TeledocRenderer::getFrameImage)
        .def("getKey", &TeledocRenderer::getKey)
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
