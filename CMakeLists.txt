cmake_minimum_required(VERSION 3.27)
project(detectMerger)

set(${PROJECT_NAME}_LIBRARIES)

set(OpenCV_DIR "C:/OpenCV10/opencv/build")

find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc)
LIST(APPEND ${PROJECT_NAME}_LIBRARIES ${OpenCV_LIBS})

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp DataStructs.h DetectMerger.h)
target_link_libraries(${PROJECT_NAME}
        PUBLIC ${${PROJECT_NAME}_LIBRARIES})
