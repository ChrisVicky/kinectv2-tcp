message(NOTICE "                       _oo0oo_                     ")
message(NOTICE "                      o8888888o                    ")
message(NOTICE "                      88\" . \"88                    ")
message(NOTICE "                      (| -_- |)                    ")
message(NOTICE "                      0\\  =  /0                    ")
message(NOTICE "                    ___/`---'\\___                  ")
message(NOTICE "                  .' \\\\|     |# '.                 ")
message(NOTICE "                 / \\\\|||  :  |||# \\                ")
message(NOTICE "                / _||||| -:- |||||- \\              ")
message(NOTICE "               |   | \\\\\\  -  #/ |   |              ")
message(NOTICE "               | \\_|  ''\\---/''  |_/ |             ")
message(NOTICE "               \\  .-\\__  '-'  ___/-. /             ")
message(NOTICE "             ___'. .'  /--.--\\  `. .'___           ")
message(NOTICE "          .\"\" '<  `.___\\_<|>_/___.' >' \"\".         ")
message(NOTICE "         | | :  `- \\`.;`\\ _ /`;.`/ - ` : | |       ")
message(NOTICE "         \\  \\ `_.   \\_ __\\ /__ _/   .-` /  /       ")
message(NOTICE "     =====`-.____`.___ \\_____/___.-`___.-'=====    ")
message(NOTICE "                       `=---='                     ")
message(NOTICE "                                                   ")
message(NOTICE "                                                   ")
message(NOTICE "     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   ")
message(NOTICE "                                                   ")
message(NOTICE "               佛祖保佑         永无BUG            ")
message(NOTICE "                                                    ")

cmake_minimum_required(VERSION 3.10)
project(txcontroller VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_FILES main.cpp)


find_package(freenect2 REQUIRED)
# find_package(OpenSSL REQUIRED)

add_subdirectory(tcp)

include_directories(${freenect2_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    tcp
    ${freenect2_LIBRARY}
)
