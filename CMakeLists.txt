cmake_minimum_required(VERSION 3.28)
project(sdlMS 
    VERSION 1.0.0 
    LANGUAGES C CXX 
    DESCRIPTION "SDL Multimedia Application Framework"
)
 
# 强化标准要求 
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
 
# LSP支持配置 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 
# 安装路径安全验证 
if(NOT DEFINED PKGDIR)
    set(PKGDIR "/usr/local" CACHE PATH "Installation directory")
endif()
 
# 编译器通用选项配置 
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>"
)
 
# 构建类型配置 
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()
 
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
 
# Release优化配置 
if(BUILD_TYPE_UPPER STREQUAL "RELEASE")
    add_compile_options(
        "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-flto=auto>"
        "$<$<CXX_COMPILER_ID:MSVC>:/GL>"
    )
    add_link_options(
        "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-flto=auto>"
        "$<$<CXX_COMPILER_ID:MSVC>:/LTCG>"
    )
endif()
 
# 架构优化选项（可选）
option(ENABLE_NATIVE_OPT "Enable architecture-specific optimizations" ON)
if(ENABLE_NATIVE_OPT)
    add_compile_options(
        "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-march=native>"
        "$<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>"
    )
endif()
 
# 第三方库管理 
add_subdirectory(3rdparty EXCLUDE_FROM_ALL)
 
# 主程序构建 
add_subdirectory(sdlms)
 
# 安装配置 
install(TARGETS sdlMS 
    RUNTIME 
        DESTINATION "${PKGDIR}/bin"
        PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE 
            GROUP_READ GROUP_EXECUTE 
            WORLD_READ WORLD_EXECUTE 
    LIBRARY 
        DESTINATION "${PKGDIR}/lib"
    ARCHIVE 
        DESTINATION "${PKGDIR}/lib"
)
 
# 配置文件安装（需要替换实际路径）
install(DIRECTORY "config/"
    DESTINATION "${PKGDIR}/etc"
    FILES_MATCHING PATTERN "*.conf"
)
 
# 创建必要目录 
install(DIRECTORY 
    DESTINATION "${PKGDIR}/logs"
    DESTINATION "${PKGDIR}/cache"
)
 
# 安装后验证 
install(CODE "
    message(STATUS \"Verifying installation...\")
    if(NOT EXISTS \"${PKGDIR}/bin/sdlMS\")
        message(FATAL_ERROR \"Installation failed: Binary missing!\")
    endif()
")
