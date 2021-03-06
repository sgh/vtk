# Setup vtkInstantiator registration for this library's classes.
VTK_MAKE_INSTANTIATOR3(vtk${KIT}Instantiator KitInstantiator_SRCS
                       "${Kit_SRCS}"
                       VTK_${UKIT}_EXPORT
                       ${VTK_BINARY_DIR} "")

ADD_LIBRARY(vtk${KIT} ${Kit_SRCS} ${Kit_EXTRA_SRCS} ${KitInstantiator_SRCS})
SET(KIT_LIBRARY_TARGETS ${KIT_LIBRARY_TARGETS} vtk${KIT})

# Allow the user to customize their build with some local options
#
SET(LOCALUSERMACRODEFINED 0)
INCLUDE (${VTK_BINARY_DIR}/${KIT}/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${VTK_SOURCE_DIR}/${KIT}/LocalUserOptions.cmake OPTIONAL)

# if we are wrapping into Tcl then add the library and extra
# source files
#
IF (VTK_WRAP_TCL)
  VTK_WRAP_TCL3(vtk${KIT}TCL KitTCL_SRCS
                "${Kit_SRCS}"
                "${Kit_TCL_EXTRA_CMDS}")
  ADD_LIBRARY(vtk${KIT}TCL ${KitTCL_SRCS} ${Kit_TCL_EXTRA_SRCS})
  SET(KIT_LIBRARY_TARGETS ${KIT_LIBRARY_TARGETS} vtk${KIT}TCL)
  TARGET_LINK_LIBRARIES (vtk${KIT}TCL vtk${KIT} ${KIT_TCL_LIBS})
  IF(NOT VTK_INSTALL_NO_LIBRARIES)
    INSTALL_TARGETS(${VTK_INSTALL_LIB_DIR} vtk${KIT}TCL)
  ENDIF(NOT VTK_INSTALL_NO_LIBRARIES)
  IF(KIT_TCL_DEPS)
    ADD_DEPENDENCIES(vtk${KIT}TCL ${KIT_TCL_DEPS})
  ENDIF(KIT_TCL_DEPS)
ENDIF (VTK_WRAP_TCL)

# if we are wrapping into Python then add the library and extra
# source files
#
IF (VTK_WRAP_PYTHON)
  # Create custom commands to generate the python wrappers for this kit.
  VTK_WRAP_PYTHON3(vtk${KIT}Python KitPython_SRCS "${Kit_SRCS}")

  # Create a shared library containing the python wrappers.  Executables
  # can link to this but it is not directly loaded dynamically as a
  # module.
  ADD_LIBRARY(vtk${KIT}PythonD ${KitPython_SRCS} ${Kit_PYTHON_EXTRA_SRCS})
  TARGET_LINK_LIBRARIES(vtk${KIT}PythonD vtk${KIT} ${KIT_PYTHON_LIBS})
  IF(NOT VTK_INSTALL_NO_LIBRARIES)
    INSTALL_TARGETS(${VTK_INSTALL_LIB_DIR} vtk${KIT}PythonD)
  ENDIF(NOT VTK_INSTALL_NO_LIBRARIES)
  SET(KIT_LIBRARY_TARGETS ${KIT_LIBRARY_TARGETS} vtk${KIT}PythonD)

  # On some UNIX platforms the python library is static and therefore
  # should not be linked into the shared library.  Instead the symbols
  # are exported from the python executable so that they can be used by
  # shared libraries that are linked or loaded.  On Windows and OSX we
  # want to link to the python libray to resolve its symbols
  # immediately.
  IF(WIN32 OR APPLE)
    TARGET_LINK_LIBRARIES (vtk${KIT}PythonD ${VTK_PYTHON_LIBRARIES})
  ENDIF(WIN32 OR APPLE)

  # Add dependencies that may have been generated by VTK_WRAP_PYTHON3 to
  # the python wrapper library.  This is needed for the
  # pre-custom-command hack in Visual Studio 6.
  IF(KIT_PYTHON_DEPS)
    ADD_DEPENDENCIES(vtk${KIT}PythonD ${KIT_PYTHON_DEPS})
  ENDIF(KIT_PYTHON_DEPS)

  # Create a python module that can be loaded dynamically.  It links to
  # the shared library containing the wrappers for this kit.
  ADD_LIBRARY(vtk${KIT}Python MODULE vtk${KIT}PythonInit.cxx)
  TARGET_LINK_LIBRARIES(vtk${KIT}Python vtk${KIT}PythonD)
ENDIF (VTK_WRAP_PYTHON)

# if we are wrapping into Java then add the library and extra
# source files
#
IF (VTK_WRAP_JAVA)
  VTK_WRAP_JAVA3(vtk${KIT}Java KitJava_SRCS "${Kit_SRCS}")
  ADD_LIBRARY(vtk${KIT}Java SHARED ${KitJava_SRCS} ${Kit_JAVA_EXTRA_SRCS})
  SET(KIT_LIBRARY_TARGETS ${KIT_LIBRARY_TARGETS} vtk${KIT}Java)
  TARGET_LINK_LIBRARIES(vtk${KIT}Java vtk${KIT} ${KIT_JAVA_LIBS})
  IF(NOT VTK_INSTALL_NO_LIBRARIES)
    INSTALL_TARGETS(${VTK_INSTALL_LIB_DIR} vtk${KIT}Java)
  ENDIF(NOT VTK_INSTALL_NO_LIBRARIES)
  IF(KIT_JAVA_DEPS)
    ADD_DEPENDENCIES(vtk${KIT}Java ${KIT_JAVA_DEPS})
  ENDIF(KIT_JAVA_DEPS)
ENDIF (VTK_WRAP_JAVA)

TARGET_LINK_LIBRARIES(vtk${KIT} ${KIT_LIBS})

IF(NOT VTK_INSTALL_NO_LIBRARIES)
  INSTALL_TARGETS(${VTK_INSTALL_LIB_DIR} vtk${KIT})
ENDIF(NOT VTK_INSTALL_NO_LIBRARIES)
IF(NOT VTK_INSTALL_NO_DEVELOPMENT)
  INSTALL_FILES(${VTK_INSTALL_INCLUDE_DIR} .h ${Kit_SRCS})
ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT)

VTK_EXPORT_KIT("${KIT}" "${UKIT}" "${Kit_SRCS}")

# If the user defined a custom macro, execute it now and pass in all the srcs
#
IF(LOCALUSERMACRODEFINED)
  LocalUserOptionsMacro( "${Kit_SRCS}"       "${Kit_EXTRA_SRCS}"
                         "${KitTCL_SRCS}"    "${Kit_TCL_EXTRA_SRCS}"
                         "${KitJava_SRCS}"   "${Kit_JAVA_EXTRA_SRCS}"
                         "${KitPython_SRCS}" "${Kit_PYTHON_EXTRA_SRCS}")
ENDIF(LOCALUSERMACRODEFINED)


# Apply user-defined properties to the library targets.
IF(VTK_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${KIT_LIBRARY_TARGETS} PROPERTIES
    ${VTK_LIBRARY_PROPERTIES}
    )
ENDIF(VTK_LIBRARY_PROPERTIES)
