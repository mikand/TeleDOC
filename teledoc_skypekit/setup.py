from distutils.core import setup, Extension
import optparse
import os, sys

p = optparse.OptionParser('%prog [options] [-- [setup_options]]')

p.add_option('--opencv2', help='Use OpenCV version 2 instead of 1', action='store_true', default=False)

try:
    idx = sys.argv.index('--')
    optargs = sys.argv[1:idx]
    argv = sys.argv[idx+1:]
except ValueError:
    optargs, argv = sys.argv[1:], []
argv.append(sys.argv[0])
sys.argv = argv

opts, args = p.parse_args(optargs)

if args:
    sys.argv = [sys.argv[0]] + args + sys.argv[1:]

opencv_macro = ("OPENCV_1", None)
opencv_lib = ['cv', 'highgui']
if opts.opencv2:
    opencv_macro = ("OPENCV_2", None)
    opencv_lib = ['opencv_core', 'opencv_highgui', 'opencv_imgproc']


module1 = Extension('teledoc',
                    sources = ['teledoc.cpp', 'tracker.cpp'],
                    include_dirs = ['skypekit/ipc/cpp', 'skypekit/ipc/cpp/VideoBuffers', '/usr/include/qt4', '/usr/include/qt4/QtGui', '/usr/include/QtGui'],
                    library_dirs = ['skypekit/interfaces/skype/cpp_embedded/build'],
                    libraries = ['QtGui','QtCore','skypekit-cyassl_lib', 'reference_opengl_lib', 'skypekit-cppwrapper_2_lib', 'boost_python'] + opencv_lib,
                    define_macros = [opencv_macro])


setup (name = 'TeleDocSkypeServer',
       version = '1.0',
       description = 'Teledoc Skype Server Python Interface',
       ext_modules = [module1])
