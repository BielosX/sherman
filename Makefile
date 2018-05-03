CC=gcc
PYTHON=python3
CFLAGS= -Wall -Wextra -pedantic
JANSSON_LIB = dependencies/jansson-2.11-build/lib/libjansson.a
JANNSON_H = -I./dependencies/jansson-2.11-build/include
INCLUDE_DIR = -I./include
DEBUG = -g
SRC_DIR = src
OBJ_DIR = obj
OBJ_FILES = $(patsubst ${SRC_DIR}/%.c, ${OBJ_DIR}/%.o,$(wildcard ${SRC_DIR}/*.c))

deploy: build_binary
	cp config.json bin

build_binary: ${OBJ_FILES}
	mkdir -p bin
	${CC} ${DEBUG} -o bin/sherman $^ ${CFLAGS} -lpthread ${JANSSON_LIB}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c build_dependencies create_obj_dir
	${CC} ${DEBUG} -c -o $@ $< ${CFLAGS} -pthread ${INCLUDE_DIR} ${JANNSON_H}

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
