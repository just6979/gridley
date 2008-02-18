PROJECT(Gridley)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE FALSE)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -O2 -ansi -pedantic)

#list all source files here
ADD_EXECUTABLE(gridley gridley.cpp)

#need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(gridley GL glut)

