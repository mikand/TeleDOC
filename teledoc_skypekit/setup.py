from distutils.core import setup, Extension

module1 = Extension('teledoc',
                    sources = ['teledoc.cpp'],
                    include_dirs = ['skypekit/ipc/cpp', 'skypekit/ipc/cpp/VideoBuffers', '/usr/include/qt4', '/usr/include/qt4/QtGui', '/usr/include/QtGui'],
                    library_dirs = ['skypekit/interfaces/skype/cpp_embedded/build'],
                    libraries = ['QtGui','QtCore','skypekit-cyassl_lib', 'reference_opengl_lib', 'skypekit-cppwrapper_2_lib', 'boost_python', 'opencv_core', 'opencv_highgui']) #'cv', 'highgui'])

setup (name = 'TeleDocSkypeServer',
       version = '1.0',
       description = 'Teledoc Skype Server Python Interface',
       ext_modules = [module1])
