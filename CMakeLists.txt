cmake_minimum_required(VERSION 2.6)

set(UDIR .)
set(ZDIR ${UDIR}/zip)
set(MIDIR ${UDIR}/misc)
set(BDIR ${UDIR}/base)

set(ZIP ${ZDIR}/zipfile.cpp ${ZDIR}/unpack_zip.cpp)
set(MISC ${MIDIR}/dirwriter.cpp)
set(BASE ${BDIR}/unpack_base.cpp ${UDIR}/auto.cpp)

set(ALL ${MANGLE} ${ZIP} ${MISC} ${BASE})

add_library(UnpackCpp ${ALL})
