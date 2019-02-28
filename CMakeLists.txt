IF(WIN32)
	set(the_description "The UbiTrack MSWindows Components")
	ut_add_component(DirectShowFrameGrabber DEPS utcore utdataflow utvision )
	
	SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	
	FIND_PACKAGE(DirectShow)
	IF(DIRECTSHOW_FOUND)
		add_definitions(-DHAVE_DIRECTSHOW)
	ENDIF(DIRECTSHOW_FOUND)

	IF(DIRECTSHOW_FOUND)
		ut_component_include_directories("src/DirectShowFrameGrabber" ${TINYXML_INCLUDE_DIR} ${LOG4CPP_INCLUDE_DIR} ${BOOSTBINDINGS_INCLUDE_DIR} ${LAPACK_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR} ${DIRECTSHOW_INCLUDE_DIRS})
		ut_glob_component_sources(HEADERS "src/DirectShowFrameGrabber/*.h" SOURCES "src/DirectShowFrameGrabber/*.cpp")
		ut_create_single_component(${DIRECTSHOW_STRMIIDS_LIBRARY})
		ut_install_utql_patterns()
  ENDIF()
ENDIF(WIN32)