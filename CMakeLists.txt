project(ubitrack_device_camera_directshow)

cmake_minimum_required(VERSION 3.4.0)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

include(GNUInstallDirs)
include(cmake/UbitrackConfig)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

IF(WIN32)
	FIND_PACKAGE(DirectShow)
	IF(DIRECTSHOW_FOUND)
		set(UBITRACK_COMPILE_DEFINITIONS ${UBITRACK_COMPILE_DEFINITIONS} "HAVE_DIRECTSHOW")
		include_directories(${DIRECTSHOW_INCLUDE_DIRS})

		SET(UBITRACK_EXTRA_LIBS ${UBITRACK_EXTRA_LIBS} ${DIRECTSHOW_STRMIIDS_LIBRARY})
		add_subdirectory(src/DirectShowFrameGrabber)

		# collect all utql patterns from module for installation
		file(GLOB _utql_patterns "doc/utql/*.xml" "doc/utql/*/*.xml"  "doc/utql/*/*/*.xml")
		foreach(pfile ${_utql_patterns})
		  string(REGEX REPLACE "^.*/doc/utql/" "" pfile2 "${pfile}")
		  GET_FILENAME_COMPONENT(fpath ${pfile2} PATH)
		  IF(fpath)
			install(FILES ${pfile} DESTINATION "${UBITRACK_UTQLPATTERN_INSTALL_DIRECTORY}/${fpath}" COMPONENT data)
		  ELSE(fpath)
		    install(FILES ${pfile} DESTINATION "${UBITRACK_UTQLPATTERN_INSTALL_DIRECTORY}" COMPONENT data)
		  ENDIF(fpath)
		endforeach()

		# install custom files
		file(GLOB _doc_files LIST_DIRECTORIES false "doc/utqlDoc/*" "doc/utqlDoc/*/*" "doc/utqlDoc/*/*/*")
		foreach(pfile ${_doc_files})
		  string(REGEX REPLACE "^.*/doc/" "" pfile2 "${pfile}")
		  GET_FILENAME_COMPONENT(fpath ${pfile2} PATH)
		  IF(fpath)
			install(FILES ${pfile} DESTINATION "${UBITRACK_DOC_INSTALL_DIRECTORY}/${fpath}" COMPONENT doc)
		  ELSE(fpath)
		    install(FILES ${pfile} DESTINATION "${UBITRACK_DOC_INSTALL_DIRECTORY}" COMPONENT doc)
		  ENDIF(fpath)
		endforeach()
	ENDIF(DIRECTSHOW_FOUND)
ENDIF(WIN32)
