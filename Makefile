SRC = $(wildcard *.cc)
OBJ = $(SRC:.cc=.o)
DEP = $(SRC:.cc=.d)
LIBS = -lstdc++fs
TARGET = papman

CXXFLAGS += -std=c++17
CXXFLAGS += -MMD -MP
DBGFLAGS += -g -ggdb3

ifeq ($(shell uname), FreeBSD)
CXX = /usr/bin/clang++
CXXFLAGS += -I /usr/local/include
LDFLAGS += -L /usr/local/lib/
endif

ifeq ($(shell  uname), Darwin)
CXX = /usr/local/bin/g++-8
CXXFLAGS += -I /usr/local/include/
LDFLAGS += -L /usr/local/lib/
endif

all: ${TARGET}

-include $(DEP)

${TARGET}: ${OBJ}
	${CXX} ${CXXFLAGS} -o $@ $^ ${LDFLAGS} ${LIBS}

clean:
	-@rm -fv ${TARGET} ${OBJ} *core

debug:
	${CXX} ${CXXFLAGS} ${DBGFLAGS} -o $@ $^ ${LDFLAGS}

distclean: clean
	-@rm -fv ${DEP}

install: ${TARGET}
	@if [ -d ${HOME}/local/bin/ ]; then\
		install -v -C papman ${HOME}/local/bin;\
	fi

.PHONY: clean debug distclean install
