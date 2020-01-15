
#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(dosescan)

#----------------------------------------------------------------------------
# Find Geant4 package
#
option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" OFF)
find_package(Geant4 REQUIRED)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)


#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(dosescan dosescan.cc ${sources} ${headers})
target_link_libraries(dosescan ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B1. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
# set(DOSESCAN_SCRIPTS
  # breast-128.img
  # mat_prmt.txt
  # scan.ini
  # )

# foreach(_script ${EXAMPLEB1_SCRIPTS})
  # configure_file(
    # ${PROJECT_SOURCE_DIR}/${_script}
    # ${PROJECT_BINARY_DIR}/${_script}
    # COPYONLY
    # )
# endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
#add_custom_target(dosescan DEPENDS dosescan)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS dosescan DESTINATION bin)


