cmake_minimum_required(VERSION 3.20)

project(dppTest VERSION 1.0 LANGUAGES CXX)

# C++17 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JSON 라이브러리 설정 (dpp 의존)
add_compile_definitions(JSON_USE_IMPLICIT_CONVERSIONS=0)

# vcpkg toolchain를 사용하면 find_package가 vcpkg 패키지를 찾습니다
# 예: dpp, openssl, mariadb

# 패키지 찾기
find_package(dpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(Threads REQUIRED)

# 실행 파일 생성
add_executable(dppTest
    src/main.cpp
    src/DiscordBotClient.cpp
    src/FileLoader.cpp
)

# UTF-8 설정 (Windows)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 라이브러리 연결
target_link_libraries(dppTest PRIVATE 
    dpp::dpp
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    unofficial::libmariadb
)
