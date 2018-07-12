SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(ubitrack_device_camera_directshow)

cmake_minimum_required(VERSION 3.4.0)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(GNUInstallDirs)
include(cmake/UbitrackConfig)

IF(WIN32)
	FIND_PACKAGE(DirectShow)
	IF(DIRECTSHOW_FOUND)
		set(UBITRACK_COMPILE_DEFINITIONS ${UBITRACK_COMPILE_DEFINITIONS} "HAVE_DIRECTSHOW")

		SET(UBITRACK_EXTRA_LIBS ${UBITRACK_EXTRA_LIBS} ${DIRECTSHOW_STRMIIDS_LIBRARY})
		add_subdirectory(src/DirectShowFrameGrabber)

		ubitrack_install_utql("doc/utql/*.xml" "doc/utql/*/*.xml"  "doc/utql/*/*/*.xml")
		ubitrack_install_doc("doc/utqlDoc/*" "doc/utqlDoc/*/*" "doc/utqlDoc/*/*/*")
	ENDIF(DIRECTSHOW_FOUND)
ENDIF(WIN32)
