from distutils.core import setup, Extension

module1 = Extension('teledoc',
                    sources = ['teledoc.cpp'],
                    include_dirs = ['/home/mikand/Desktop/sdp-distro-embedded-skypekit_4.3.1.17_1899690/ipc/cpp', '/home/mikand/Desktop/sdp-distro-embedded-skypekit_4.3.1.17_1899690/ipc/cpp/VideoBuffers', '/usr/include/qt4', '/usr/include/qt4/QtGui'],
                    library_dirs = ['/home/mikand/Desktop/sdp-distro-embedded-skypekit_4.3.1.17_1899690/interfaces/skype/cpp_embedded/build'],
                    libraries = ['QtGui','QtCore','skypekit-cyassl_lib'])

setup (name = 'TeleDocSkypeServer',
       version = '1.0',
       description = 'Teledoc Skype Server Python Interface',
       ext_modules = [module1])
