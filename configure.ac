AC_INIT([BDReader], [1.01], [kennyyy@users.sf.net])
AM_INIT_AUTOMAKE([foreign -Wall])

#Add C++ as a language (for headers testing)
AC_LANG_PUSH([C++])

#Check headers
AC_CHECK_HEADERS([FL/Fl.H Fl.H Fl.h], [break;])
AC_CHECK_HEADERS([opencv2/opencv.hpp opencv.hpp], [break;])
AC_CHECK_HEADERS([mupdf/fitz.h], [break;])
AC_CHECK_HEADERS([archive_entry.h], [break;])

#Libraries check
#fltk
AC_CHECK_LIB([fltk], [main], [], [
  AC_MSG_ERROR([unable to find fltk library.])
])
AC_CHECK_LIB([Xft], [XftFontOpen], [], [
  AC_MSG_ERROR([unable to find Xft library.])
])
AC_CHECK_LIB([Xinerama], [XineramaQueryScreens], [], [
  AC_MSG_ERROR([unable to find Xinerama library.])
])
#mupdf
AC_SEARCH_LIBS([fz_free], [mupdf], [], [
  AC_MSG_ERROR([unable to find fz_free() function (mupdf library).])
])
AC_SEARCH_LIBS([pdf_new_js], [mupdf-js-none], [], [
  AC_MSG_ERROR([unable to find pdf_new_js() function (mupdf-js-none library).])
])
AC_CHECK_LIB([jpeg], [main], [], [
  AC_MSG_ERROR([unable to find jpeg library.])
])
AC_CHECK_LIB([openjpeg], [main], [], [
  AC_MSG_ERROR([unable to find openjpeg library.])
])
AC_CHECK_LIB([jbig2dec], [main], [], [
  AC_MSG_ERROR([unable to find jbig2dec library.])
])
#opencv
AC_CHECK_LIB([opencv_core], [main], [], [
  AC_MSG_ERROR([unable to find opencv_core library.])
])
AC_CHECK_LIB([opencv_imgproc], [main], [], [
  AC_MSG_ERROR([unable to find opencv_imgproc library.])
])
AC_CHECK_LIB([opencv_highgui], [main], [], [
  AC_MSG_ERROR([unable to find opencv_highgui library.])
])
#libarchive
AC_CHECK_LIB([archive], [main], [], [
  AC_MSG_ERROR([unable to find libarchive library.])
])

#Compiler check
AC_PROG_CXX

#Output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
