CC=gcc
PYTHON=python3
CFLAGS= -Wall -Wextra -pedantic
JANSSON_LIB = dependencies/jansson-2.11-build/lib/libjansson.a
JANNSON_H = -I./dependencies/jansson-2.11-build/include
DEBUG = -g

deploy: build_main
	cp config.json bin

build_main: build_dependencies
	mkdir -p bin
	${CC} ${DEBUG} -o bin/sherman src/main.c ${CFLAGS} -lpthread ${JANSSON_LIB} ${JANNSON_H}

build_dependencies:
	${PYTHON} download_dep.py

clean:
	rm -rf bin/sherman
	rm -rf bin
