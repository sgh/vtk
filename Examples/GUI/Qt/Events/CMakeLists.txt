
INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
IF (USE_VTK_FILE)
  INCLUDE(${USE_VTK_FILE})
ENDIF (USE_VTK_FILE)

INCLUDE (${CMAKE_ROOT}/Modules/FindQt.cmake)

SET (SRCS
  main.cxx
  vtkEventQtSlotConnect.cxx
)
SET (MOC_HDRS
  vtkEventQtSlotConnect.h
)

INCLUDE_DIRECTORIES(
        ${QT_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

QT_WRAP_UI( qtevents UI_HDRS UI_SRCS GUI.ui )
QT_WRAP_CPP( qtevents MOC_SRCS ${MOC_HDRS} )

# It's nice to have the ui in the windows project file...just double click on it
# and designer comes up on that ui file :)
#IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  #SET (SimpleViewSrcs ${SimpleViewSrcs} uiSimpleView.ui)
#ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
#IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  #SET (SimpleViewSrcs ${SimpleViewSrcs} uiSimpleView.ui)
#ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

ADD_EXECUTABLE( qtevents ${SRCS} ${UI_SRCS} ${MOC_SRCS})

# note: if you use Mesa in VTK and get link errors about opengl symbols
#       they are coming from Qt.  Either link OpenGL in, configure Qt to
#       dlopen the GL library, or configure Qt without OpenGL support.


TARGET_LINK_LIBRARIES( qtevents
  vtkGUISupport
  ${QT_QT_LIBRARY}
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
)

