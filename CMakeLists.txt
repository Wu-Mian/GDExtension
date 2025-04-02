cmake_minimum_required(VERSION 3.12)
project(DiffDetectorGDExtension)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Godot C++ 绑定路径（假设godot-cpp目录存在于项目根目录中）
set(GODOT_CPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp")

# 包含godot-cpp的CMake配置
add_subdirectory(${GODOT_CPP_PATH})

# OpenCV-mobile路径
set(OPENCV_MOBILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/opencv-mobile")

# LiteRT路径
set(LITERT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/litert")

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GODOT_CPP_PATH}/include
    ${GODOT_CPP_PATH}/gen/include
    ${OPENCV_MOBILE_PATH}/include
    ${LITERT_PATH}/include
)

# 源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.c"
)

# 目标平台检测
if(ANDROID)
    # Android平台配置
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    set(PLATFORM_NAME "android")
elseif(IOS)
    # iOS平台配置  
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    set(PLATFORM_NAME "ios") 
else()
    message(FATAL_ERROR "仅支持Android和iOS平台")
endif()

# 设置库输出目录
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}")

# 链接库
target_link_libraries(${PROJECT_NAME}
    godot-cpp
    ${OPENCV_MOBILE_PATH}/lib/${PLATFORM_NAME}/libopencv_mobile.a
    ${LITERT_PATH}/lib/${PLATFORM_NAME}/liblitert.a
)

# 复制资源文件到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${LIBRARY_OUTPUT_PATH}/assets
) 