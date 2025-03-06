cmake_minimum_required(VERSION 3.28)

# 在顶层CMakeLists.txt 添加安装路径配置 
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/PKGDIR" CACHE PATH "Install directory")
endif()
message(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_STANDARD 23)

# Add compile information for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # 启用lto -flto=auto
        # 去掉调试符号
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=auto")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        # 对本机cpu生成特定优化代码,可能存在兼容性问题
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
    endif()
endif()

add_subdirectory(3rdparty EXCLUDE_FROM_ALL)
add_subdirectory(sdlms)

# 在sdlms子项目的CMakeLists中确保安装目标 
install(TARGETS sdlms 
    RUNTIME DESTINATION bin 
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
)
 
# 创建必要的安装目录 
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sdlms)

