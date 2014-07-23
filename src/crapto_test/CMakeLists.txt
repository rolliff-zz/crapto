LIST(APPEND SOURCE
	crapto_test.cpp
	crapto_test.h
	crc.h
	crc.cpp
	buffer_test.cpp
	)
INCLUDE_DIRECTORIES(${ROOT}/src/public ${ROOT}/external/tcdist)
ADD_EXECUTABLE(crapto_test ${SOURCE})
TARGET_LINK_LIBRARIES(crapto_test ${LIBRARY_OUTPUT_PATH}/${CONF}/crapto${CMAKE_LINK_LIBRARY_SUFFIX})
TARGET_LINK_LIBRARIES(crapto_test ${ROOT}/external/tcdist/tc.lib)
ADD_DEPENDENCIES(crapto_test crapto)
