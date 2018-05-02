CC=gcc
PYTHON=python3
CFLAGS= -Wall -Wextra -pedantic

build_main: build_dependencies
	${CC} -o sherman main.c ${CFLAGS}

build_dependencies:
	${PYTHON} download_dep.py
