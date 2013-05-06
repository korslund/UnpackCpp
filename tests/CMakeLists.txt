cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("../")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES})

set(MDIR ../mangle)
set(UDIR ..)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(UNPACK ${MANGLE} ${UDIR}/base/unpack_base.cpp ${UDIR}/misc/dirwriter.cpp)

add_executable(dummy1_test dummy1_test.cpp ${UNPACK})
target_link_libraries(dummy1_test ${LIBS})

#add_executable(zip_test zip_test.cpp ${UNPACK})
#target_link_libraries(zip_test ${LIBS})

#add_executable(auto_test auto_test.cpp ${UNPACK})
#target_link_libraries(auto_test ${LIBS})
