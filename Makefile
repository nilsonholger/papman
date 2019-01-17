SRC = $(wildcard *.cc)
OBJ = $(SRC:.cc=.o)
DEP = $(SRC:.cc=.d)
LIBS = -lstdc++fs
TARGET = papman

CXXFLAGS += -std=c++17
CXXFLAGS += -g -ggdb3 -MMD -MP
INSTALL = install --compare --verbose

ifeq ($(shell uname), FreeBSD)
CXX = /usr/bin/clang++
CXXFLAGS += -I /usr/local/include
LDFLAGS += -L /usr/local/lib/
endif

ifeq ($(shell  uname), Darwin)
CXX = /usr/local/bin/g++-8
CXXFLAGS += -I /usr/local/include/
LDFLAGS += -L /usr/local/lib/
INSTALL = install -Cv
endif

all: ${TARGET}

-include $(DEP)

${TARGET}: ${OBJ}
	${CXX} ${CXXFLAGS} -o $@ $^ ${LDFLAGS} ${LIBS}

clean:
	-@rm -fv ${TARGET} ${OBJ} *core

distclean: clean
	-@rm -rfv ${DEP} ${TARGET}.dSYM

install: ${TARGET}
	@if [ -d ${HOME}/local/bin/ ]; then\
		strip ${TARGET};\
		${INSTALL} ${TARGET} ${HOME}/local/bin;\
	fi

.PHONY: clean distclean install
