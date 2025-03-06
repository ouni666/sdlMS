cmake_minimum_required(VERSION 3.28)

# 在顶层CMakeLists.txt 添加安装路径配置 
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/PKGDIR" CACHE PATH "Install directory")
endif()
message(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}")

project(sdlMS)

set(CMAKE_CXX_STANDARD 23)

# Add compile information for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # 启用LTO需要配套设置 
        include(CheckIPOSupported)
        check_ipo_supported(RESULT ipo_supported)
        if(ipo_supported)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
        
        # 跨平台优化参数调整 
        if(NOT CMAKE_CROSSCOMPILING)
            add_compile_options(-march=native)
        else 
            add_compile_options(-mtune=generic)
        endif()
        
        # 符号剥离参数优化 
        add_link_options("LINKER:-s")
    endif()
endif()

# 确保子目录存在且包含有效CMakeLists 
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty AND 
   EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sdlms)
    add_subdirectory(3rdparty)
    add_subdirectory(sdlms)
else()
    message(FATAL_ERROR "Missing required subdirectories: 3rdparty/ and sdlms/")
endif()

# 在sdlms子项目的CMakeLists中确保安装目标 
install(TARGETS sdlms 
    RUNTIME DESTINATION bin 
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
)
 
# 创建必要的安装目录 
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sdlms)

