cmake_minimum_required(VERSION 3.28) 
project(sdlMS) 
set(CMAKE_CXX_STANDARD 23) 
 
# Add compile information for LSP 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
 
# 假设 PKGDIR 已经定义，若未定义可以通过命令行传入 
if(NOT DEFINED PKGDIR) 
    message(FATAL_ERROR "PKGDIR is not defined. Please set it using -DPKGDIR=/path/to/dir") 
endif() 
 
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
 
# 假设你要安装可执行文件和一些配置文件，以下是示例 
# 安装可执行文件 
install(TARGETS sdlMS # 这里假设 sdlMS 是你的可执行目标名称 
        RUNTIME DESTINATION "${PKGDIR}/bin" # 可执行文件安装到 PKGDIR/bin 目录 
) 
 
# 安装配置文件 
install(FILES path/to/your/config/file.conf  # 替换为实际的配置文件路径 
        DESTINATION "${PKGDIR}/etc" # 配置文件安装到 PKGDIR/etc 目录 
) 
