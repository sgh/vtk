# Defined local options to add to VTK Tcl interpreter
# 
# Example of such options are:
#   VTK_EXTRA_TCL_WRAPPING_CODE which should define extra code that will be
#   added to vtkTkAppInitConfigure.h. This code may include something like
#
#   
#   extern "C" int Vtkmyspecialtcl_Init(Tcl_Interp *interp);
#   #define VTK_EXTRA_TCL_INIT \
#     if (Vtkmyspecialtcl_Init(interp) == TCL_ERROR) \
#       {
#       return TCL_ERROR;
#       }
#
#   and 
#
#   SET(VTK_EXTRA_TCL_LIBRARIES ${VTK_EXTRA_TCL_LIBRARIES} vtkMySpecialTCL)
INCLUDE (${VTK_BINARY_DIR}/Wrapping/Tcl/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${VTK_SOURCE_DIR}/Wrapping/Tcl/LocalUserOptions.cmake OPTIONAL)

CONFIGURE_FILE(
  ${VTK_SOURCE_DIR}/Wrapping/Tcl/vtkTkAppInitConfigure.h.in
  ${VTK_BINARY_DIR}/Wrapping/Tcl/vtkTkAppInitConfigure.h
  @ONLY IMMEDIATE
)

IF (NOT TCL_LIBRARY)
  MESSAGE("VTK_WRAP_TCL is ON, but the Tcl library was not found.  Please set TCL_LIBRARY." "Error")
ENDIF (NOT TCL_LIBRARY)

IF (WIN32)
  IF (NOT BORLAND)
    IF(NOT CYGWIN)
      CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/vtk.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/resources/vtk.rc)
      INCLUDE_DIRECTORIES(
        "${VTK_TK_RESOURCES_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "${CMAKE_CURRENT_BINARY_DIR}/resources"
        )
      SET(VTK_EXE_RESOURCE_FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/resources/vtk.rc")
    ENDIF(NOT CYGWIN)
  ENDIF (NOT BORLAND)
ENDIF (WIN32)

# Create the vtk Tcl wrapper executable.
ADD_EXECUTABLE(vtk vtkTkAppInit.cxx ${VTK_EXE_RESOURCE_FILES})
TARGET_LINK_LIBRARIES (vtk
  vtkCommonTCL
  vtkFilteringTCL
  vtkGraphicsTCL
  vtkImagingTCL
  vtkIOTCL
)

IF(VTK_USE_CARBON)
  FIND_PROGRAM(VTK_APPLE_RESOURCE Rez /Developer/Tools)
  IF(VTK_APPLE_RESOURCE)
    ADD_CUSTOM_COMMAND(
      SOURCE vtk
      COMMAND ${VTK_APPLE_RESOURCE}
      ARGS Carbon.r -o ${VTK_EXECUTABLE_DIR}/vtk
      TARGET vtk
      )
  ENDIF(VTK_APPLE_RESOURCE)
ENDIF(VTK_USE_CARBON)

IF (VTK_USE_RENDERING)
  TARGET_LINK_LIBRARIES(vtk vtkRenderingTCL)
  TARGET_LINK_LIBRARIES(vtk vtkVolumeRenderingTCL)
  TARGET_LINK_LIBRARIES(vtk vtkHybridTCL)
  TARGET_LINK_LIBRARIES(vtk vtkWidgetsTCL)
ENDIF (VTK_USE_RENDERING)

IF (VTK_USE_PARALLEL)
  TARGET_LINK_LIBRARIES(vtk vtkParallelTCL)
ENDIF (VTK_USE_PARALLEL)

# Link to extra TCL libraries
FOREACH(lib ${VTK_EXTRA_TCL_LIBRARIES})
  TARGET_LINK_LIBRARIES(vtk ${lib})
ENDFOREACH(lib)

# Setup install location for vtk tcl scripts.
SET(VTK_TCL_INSTALL_DIR ${VTK_INSTALL_TCL_DIR}/tcl)

IF(NOT VTK_INSTALL_NO_RUNTIME)
  INSTALL_TARGETS(${VTK_INSTALL_BIN_DIR} vtk)
ENDIF(NOT VTK_INSTALL_NO_RUNTIME)

# Create the pvtk Tcl wrapper executable with MPI support.
IF (VTK_USE_PARALLEL)
  IF (VTK_USE_MPI)
    ADD_EXECUTABLE(pvtk vtkParaTkAppInit.cxx ${VTK_EXE_RESOURCE_FILES})
    IF(VTK_USE_CARBON)
      FIND_PROGRAM(VTK_APPLE_RESOURCE Rez /Developer/Tools)
      IF(VTK_APPLE_RESOURCE)
        ADD_CUSTOM_COMMAND(
          SOURCE pvtk
          COMMAND ${VTK_APPLE_RESOURCE}
          ARGS Carbon.r -o ${VTK_EXECUTABLE_DIR}/pvtk
          TARGET pvtk
          )
      ENDIF(VTK_APPLE_RESOURCE)
    ENDIF(VTK_USE_CARBON)

    TARGET_LINK_LIBRARIES (pvtk
      vtkCommonTCL
      vtkFilteringTCL
      vtkGraphicsTCL
      vtkImagingTCL
      vtkIOTCL
      vtkParallelTCL
    )

    IF (VTK_USE_RENDERING)
      TARGET_LINK_LIBRARIES(pvtk vtkHybridTCL)
      TARGET_LINK_LIBRARIES(pvtk vtkWidgetsTCL)
      TARGET_LINK_LIBRARIES(pvtk vtkVolumeRenderingTCL)
    ENDIF (VTK_USE_RENDERING)

    IF(NOT VTK_INSTALL_NO_RUNTIME)
      INSTALL_TARGETS(${VTK_INSTALL_BIN_DIR} pvtk)
    ENDIF(NOT VTK_INSTALL_NO_RUNTIME)
  ENDIF (VTK_USE_MPI)
ENDIF (VTK_USE_PARALLEL)

# Process all packages
SUBDIRS(vtk vtkbase vtkcommon vtkfiltering vtkgraphics vtkimaging vtkinteraction vtkio vtktesting)

IF(NOT VTK_INSTALL_NO_DOCUMENTATION)
  INSTALL_FILES(${VTK_TCL_INSTALL_DIR} "" README)
ENDIF(NOT VTK_INSTALL_NO_DOCUMENTATION)
IF(NOT VTK_INSTALL_NO_DEVELOPMENT)
  INSTALL_FILES(${VTK_TCL_INSTALL_DIR} .c vtktcl)
ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT)

SET(VTK_TCL_RENDERING_CONFIG "")
SET(VTK_TCL_VOLUMERENDERING_CONFIG "")
SET(VTK_TCL_HYBRID_CONFIG "")
SET(VTK_TCL_WIDGETS_CONFIG "")
SET(VTK_TCL_PARALLEL_CONFIG "")

IF (VTK_USE_PARALLEL)
  SET(VTK_TCL_PARALLEL_CONFIG "Parallel")
  SUBDIRS(vtkparallel)
ENDIF (VTK_USE_PARALLEL)

IF (VTK_USE_RENDERING)
  SET(VTK_TCL_WIDGETS_CONFIG "Widgets")
  SUBDIRS(vtkwidgets)
  SET(VTK_TCL_HYBRID_CONFIG "Hybrid")
  SUBDIRS(vtkhybrid)
  SET(VTK_TCL_VOLUMERENDERING_CONFIG "VolumeRendering")
  SUBDIRS(vtkvolumerendering)
  SET(VTK_TCL_RENDERING_CONFIG "Rendering")
  SUBDIRS(vtkrendering)
ENDIF (VTK_USE_RENDERING)

# Configure the Tcl package index files for the build tree.
SET(VTK_TCL_SCRIPT_DIR "${VTK_SOURCE_DIR}/Wrapping/Tcl")

SET(VTK_TCL_LIBNAME_PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")
SET(VTK_TCL_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})

IF(VTK_TCL_CONFIGURATION_TYPES)
  FOREACH(config ${VTK_TCL_CONFIGURATION_TYPES})
    SET(VTK_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}/${config}")
    CONFIGURE_FILE(${VTK_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
                   ${VTK_BINARY_DIR}/Wrapping/Tcl/${config}/pkgIndex.tcl
                   @ONLY IMMEDIATE)
  ENDFOREACH(config)
ELSE(VTK_TCL_CONFIGURATION_TYPES)
  SET(VTK_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  CONFIGURE_FILE(${VTK_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
                 ${VTK_BINARY_DIR}/Wrapping/Tcl/pkgIndex.tcl
                 @ONLY IMMEDIATE)
ENDIF(VTK_TCL_CONFIGURATION_TYPES)

# Configure the Tcl package index file for the install tree.
SET(VTK_TCL_SCRIPT_DIR "[file dirname [info script]]/tcl")
IF(UNIX)
  SET(VTK_TCL_LIBRARY_DIR "[file dirname [file dirname [info script]]]")
ELSE(UNIX)
  SET(VTK_TCL_LIBRARY_DIR
      "[file join [file dirname [file dirname [file dirname [info script]]]] bin]")
ENDIF(UNIX)
CONFIGURE_FILE(${VTK_SOURCE_DIR}/Wrapping/Tcl/pkgIndex.tcl.in
               ${VTK_BINARY_DIR}/Wrapping/Tcl/Install/Hide/pkgIndex.tcl
               @ONLY IMMEDIATE)
IF(NOT VTK_INSTALL_NO_RUNTIME)
  INSTALL_FILES(${VTK_INSTALL_TCL_DIR} FILES
                ${VTK_BINARY_DIR}/Wrapping/Tcl/Install/Hide/pkgIndex.tcl)
ENDIF(NOT VTK_INSTALL_NO_RUNTIME)

# Allow the user to customize their build with some local options
#
INCLUDE (${VTK_BINARY_DIR}/Wrapping/Tcl/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${VTK_SOURCE_DIR}/Wrapping/Tcl/LocalUserOptions.cmake OPTIONAL)
