LIST(APPEND SOURCE
	crapto_runtime.c
	crapto_array.c
	crapto_array.h
	crapto_buffer.c
	crapto_buffer.h
	crapto_file.c
	crapto_file.h
	crapto_list.c
	crapto_list.h
	crapto_private.h
	${ROOT}/src/public/crapto.h
	)

INCLUDE_DIRECTORIES(${ROOT}/src/public)
ADD_LIBRARY(libcrapto STATIC ${SOURCE})
