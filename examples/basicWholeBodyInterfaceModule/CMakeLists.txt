# Copyright (C) 2014 Fondazione Istituto Italiano di Tecnologia
# Author: Silvio Traversaro
# CopyPolicy: Released under the terms of the GNU GPL v2.0 or any later version.

cmake_minimum_required(VERSION 2.8.11)
SET(PROJECTNAME basicWholeBodyInterfaceModule)
PROJECT(${PROJECTNAME})

find_package(YARP REQUIRED)
find_package(YCM REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(iDynTree REQUIRED)
find_package(yarpWholeBodyInterface REQUIRED)

#use yarp and ycm find package scripts
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${YARP_MODULE_PATH})
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${YCM_MODULE_PATH})

FILE(GLOB folder_source ./src/*.cpp)
FILE(GLOB folder_header ./src/*.h)

SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})

# new "data" installation - available with yarp2.4
include(YarpInstallationHelpers)
yarp_configure_external_installation(codyco)

ADD_EXECUTABLE(${PROJECTNAME} ${folder_source} ${folder_header})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src
                    ${YARP_INCLUDE_DIRS}
                    ${yarpWholeBodyInterface_INCLUDE_DIRS}
                    ${EIGEN3_INCLUDE_DIR}
                    )

TARGET_LINK_LIBRARIES(${PROJECTNAME} ${YARP_LIBRARIES}
                                     ${ICUB_LIBRARIES}
                                     ${iDynTree_LIBRARIES}
                                     ${yarpWholeBodyInterface_LIBRARIES})

INSTALL(TARGETS ${PROJECTNAME} DESTINATION bin)

add_subdirectory(app)
