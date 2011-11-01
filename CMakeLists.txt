########################################################
# cmake file for building Marlin example Package
# @author Jan Engels, Desy IT
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
########################################################


# project name
PROJECT( ForwardTracking )


# project version
SET( ${PROJECT_NAME}_VERSION_MAJOR 0 )
SET( ${PROJECT_NAME}_VERSION_MINOR 1 )
SET( ${PROJECT_NAME}_VERSION_PATCH 0 )



### DEPENDENCIES ############################################################

FIND_PACKAGE( ILCUTIL REQUIRED COMPONENTS ILCSOFT_CMAKE_MODULES ILCTEST )

# load default settings from ILCSOFT_CMAKE_MODULES
INCLUDE( ilcsoft_default_settings )

FIND_PACKAGE( Marlin 1.0 REQUIRED ) # minimum required Marlin version
INCLUDE_DIRECTORIES( ${Marlin_INCLUDE_DIRS} )
LINK_LIBRARIES( ${Marlin_LIBRARIES} )
ADD_DEFINITIONS( ${Marlin_DEFINITIONS} )

FIND_PACKAGE( MarlinUtil 1.0 REQUIRED ) # minimum required MarlinUtil version
INCLUDE_DIRECTORIES( ${MarlinUtil_INCLUDE_DIRS} )
LINK_LIBRARIES( ${MarlinUtil_LIBRARIES} )
ADD_DEFINITIONS( ${MarlinUtil_DEFINITIONS} )

FIND_PACKAGE( MarlinTrk REQUIRED ) 
INCLUDE_DIRECTORIES( ${MarlinTrk_INCLUDE_DIRS} )
LINK_LIBRARIES( ${MarlinTrk_LIBRARIES} )
ADD_DEFINITIONS( ${MarlinTrk_DEFINITIONS} )

FIND_PACKAGE( ROOT REQUIRED ) 
INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIRS} )
LINK_LIBRARIES( ${ROOT_LIBRARIES} )
ADD_DEFINITIONS( ${ROOT_DEFINITIONS} )

FIND_PACKAGE( GSL REQUIRED ) 
INCLUDE_DIRECTORIES( ${GSL_INCLUDE_DIRS} )
LINK_LIBRARIES( ${GSL_LIBRARIES} )
ADD_DEFINITIONS( ${GSL_DEFINITIONS} )

# optional package
FIND_PACKAGE( RAIDA )
IF( RAIDA_FOUND )
    INCLUDE_DIRECTORIES( ${RAIDA_INCLUDE_DIRS} )
    LINK_LIBRARIES( ${RAIDA_LIBRARIES} )
    ADD_DEFINITIONS( "-DMARLIN_USE_AIDA" )
    MESSAGE( STATUS "RAIDA -- found" )
ELSE()
    MESSAGE( STATUS "RAIDA -- not found" )
ENDIF()





FIND_PACKAGE( CLHEP REQUIRED )
IF( CLHEP_FOUND )
        INCLUDE_DIRECTORIES( ${CLHEP_INCLUDE_DIRS} )
        LINK_LIBRARIES( ${CLHEP_LIBRARIES} )
        ADD_DEFINITIONS( ${CLHEP_DEFINITIONS} )
        MESSAGE( STATUS "CLHEP -- found" )
ELSE()
        MESSAGE( STATUS "CLHEP -- not found" )
ENDIF()


### DOCUMENTATION ###########################################################

OPTION( INSTALL_DOC "Set to OFF to skip build/install Documentation" OFF )

IF( INSTALL_DOC AND EXISTS "${PROJECT_SOURCE_DIR}/doc/CMakeLists.txt" )
    ADD_SUBDIRECTORY( ./doc )
ENDIF()



### LIBRARY #################################################################

# definitions to pass to the compiler
ADD_DEFINITIONS( "-Wall -ansi -pedantic" )
ADD_DEFINITIONS( "-Wno-long-long" )

# include directories
INCLUDE_DIRECTORIES( ./include/FTrack ./include/TrackingFeedback ./include/TrackPicker ./include/NeuralNetwork ./include/FTDNoise ./include/Tools ./include/Analysis ./include/Criteria )
#INSTALL_DIRECTORY( ./include DESTINATION . FILES_MATCHING PATTERN "*.h" )

# add library
AUX_SOURCE_DIRECTORY( ./src/FTrack library_sources )  
AUX_SOURCE_DIRECTORY( ./src/TrackingFeedback library_sources )
AUX_SOURCE_DIRECTORY( ./src/TrackPicker library_sources )
AUX_SOURCE_DIRECTORY( ./src/NeuralNetwork library_sources )
AUX_SOURCE_DIRECTORY( ./src/FTDNoise library_sources )
AUX_SOURCE_DIRECTORY( ./src/Tools library_sources )
AUX_SOURCE_DIRECTORY( ./src/Analysis library_sources )
AUX_SOURCE_DIRECTORY( ./src/Criteria library_sources )



ADD_SHARED_LIBRARY( ${PROJECT_NAME} ${library_sources} )
INSTALL_SHARED_LIBRARY( ${PROJECT_NAME} DESTINATION lib )


ADD_EXECUTABLE( QuantileAnalyser ./src/Executables/QuantileAnalyser.cc )
TARGET_LINK_LIBRARIES( QuantileAnalyser ${PROJECT_NAME} )

ADD_EXECUTABLE( CritRunner ./src/Executables/CritRunner.cc )
TARGET_LINK_LIBRARIES( CritRunner ${PROJECT_NAME} )


### TESTING #################################################################

MACRO( ADD_UNIT_TEST _name )
    IF( BUILD_TESTING )
        ADD_EXECUTABLE( test_${_name} ${ARGN} )
    ELSE()
        ADD_EXECUTABLE( test_${_name} EXCLUDE_FROM_ALL ${ARGN} )
    ENDIF()        
    TARGET_LINK_LIBRARIES( test_${_name} ${PROJECT_NAME} )
    GET_TARGET_PROPERTY( _executable test_${_name} LOCATION )
    ADD_TEST( t_${_name} ${_executable} )
ENDMACRO()

INCLUDE_DIRECTORIES( ${ILCTEST_INCLUDE_DIRS} )


ADD_UNIT_TEST( AutCode ./src/testing/test_AutCode.cc )
SET_TESTS_PROPERTIES( t_AutCode PROPERTIES FAIL_REGULAR_EXPRESSION "TEST_FAILED" )
SET_TESTS_PROPERTIES( t_AutCode PROPERTIES PASS_REGULAR_EXPRESSION "TEST_PASSED" )

ADD_UNIT_TEST( simple_circle ./src/testing/test_simple_circle.cc )
SET_TESTS_PROPERTIES( t_simple_circle PROPERTIES FAIL_REGULAR_EXPRESSION "TEST_FAILED" )
SET_TESTS_PROPERTIES( t_simple_circle PROPERTIES PASS_REGULAR_EXPRESSION "TEST_PASSED" )
SET_TESTS_PROPERTIES( t_simple_circle PROPERTIES WILL_FAIL TRUE )




# display some variables and write them to cache
DISPLAY_STD_VARIABLES()

