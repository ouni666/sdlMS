# 在顶层CMakeLists.txt  添加安装路径配置 
if(NOT DEFINED CMAKE_INSTALL_PREFIX) 
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/PKGDIR" CACHE PATH "Install directory") 
endif() 
message(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}") 
 
# 包含头文件目录 
include_directories("${CMAKE_CURRENT_SOURCE_DIR}") 
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/entt/src") 
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/SDL/include") 
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp") 
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/include") 
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/mio/include") 
 
# ffmpeg 相关路径配置 
include_directories("C:/ffmpeg-master-latest-win64-gpl-shared/include") 
link_directories("C:/ffmpeg-master-latest-win64-gpl-shared/lib") 
 
include_directories("C:/FFMPEG-WEB/include") 
link_directories("C:/FFMPEG-WEB/lib") 
 
# 查找源文件 
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp) 
 
# 添加可执行文件，只保留一次定义 
add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp  ${SOURCE_FILES}) 
 
# 链接库 
target_link_libraries(sdlMS PRIVATE SDL3::SDL3) 
target_link_libraries(sdlMS PRIVATE wzlib) 
target_link_libraries(sdlMS PRIVATE freetype) 
target_link_libraries(sdlMS PRIVATE EnTT) 
target_link_libraries(sdlMS PRIVATE avformat avcodec avutil swscale swresample) 
 
# 针对不同平台的配置 
if(WIN32) 
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") 
        add_definitions(-DNOMINMAX) 
    endif() 
    set_target_properties(sdlMS PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} 
    ) 
endif() 
 
if(APPLE) 
    set_target_properties(sdlMS PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin 
    ) 
endif() 
 
# 设置 Emscripten 编译选项 
if(EMSCRIPTEN) 
    set(CMAKE_EXECUTABLE_SUFFIX ".html") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sALLOW_MEMORY_GROWTH -sUSE_ZLIB -sSTACK_SIZE=32MB") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sINITIAL_MEMORY=64MB") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sFETCH=1") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sASYNCIFY=1") 
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -flto") 
    set_target_properties(sdlMS PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} 
    ) 
endif() 
 
# 安装目标 
install(TARGETS sdlMS DESTINATION bin) 
