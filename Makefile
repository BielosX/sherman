CC=gcc
PYTHON=python3
CFLAGS= -Wall -Wextra -pedantic
JANSSON_LIB = dependencies/jansson-2.11-build/lib/libjansson.a
JANNSON_H = -I./dependencies/jansson-2.11-build/include
GLIB_H = -I./dependencies/glib-2.56.1-build/include/glib-2.0
GMODULE_LIB = dependencies/glib-2.56.1-build/lib/libgmodule-2.0.a
GLIB_LIB = dependencies/glib-2.56.1-build/lib/libglib-2.0.a
GLIB_CONFIG_H = -I./dependencies/glib-2.56.1-build/lib/glib-2.0/include
INCLUDE_DIR = -I./include
DEBUG = -g
SRC_DIR = src
OBJ_DIR = obj
OBJ_FILES = $(patsubst ${SRC_DIR}/%.c, ${OBJ_DIR}/%.o,$(wildcard ${SRC_DIR}/*.c))

deploy: build_binary
	cp config.json bin

build_binary: ${OBJ_FILES}
	mkdir -p bin
	${CC} ${DEBUG} -o bin/sherman $^ ${CFLAGS} -lpthread ${JANSSON_LIB} ${GMODULE_LIB} ${GLIB_LIB}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c build_dependencies create_obj_dir
	${CC} ${DEBUG} -c -o $@ $< ${CFLAGS} -pthread ${INCLUDE_DIR} ${JANNSON_H} ${GLIB_H} ${GLIB_CONFIG_H}

build_client:
	mkdir -p bin
	${CC} ${DEBUG} -o bin/client client/main.c ${CFLAGS} -lpthread ${INCLUDE_DIR}

create_obj_dir:
	mkdir -p ${OBJ_DIR}

build_dependencies:
	${PYTHON} download_dep.py

valgrind:
	valgrind bin/sherman bin/config.json

clean:
	rm -rf bin/sherman
	rm -rf bin
	rm -rf src/*.o
	rm -rf obj
