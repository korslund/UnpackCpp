cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(ZLIB)

include_directories("../../")
include_directories(${Boost_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

set(MDIR ../../mangle)
set(UDIR ../..)
set(ZDIR ${UDIR}/zip)
set(MIDIR ${UDIR}/misc)
set(BDIR ${UDIR}/base)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(ZIP ${ZDIR}/zipfile.cpp ${ZDIR}/unpack_zip.cpp)
set(MISC ${MIDIR}/dirwriter.cpp)
set(BASE ${BDIR}/unpack_base.cpp)

set(ALL ${MANGLE} ${ZIP} ${MISC} ${BASE})

add_executable(zipfile_test zipfile_test.cpp ${ALL})
target_link_libraries(zipfile_test ${LIBS})

add_executable(zip_test zip_test.cpp ${ALL})
target_link_libraries(zip_test ${LIBS})
