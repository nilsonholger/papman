SRC = $(wildcard *.cc)
OBJ = $(SRC:.cc=.o)
DEP = $(SRC:.cc=.d)
LIBS = -lstdc++fs
TARGET = papman

CXXFLAGS += -std=c++17
CXXFLAGS += -g -ggdb3 -MMD -MP

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

distclean: clean
	-@rm -fv ${DEP}

install: ${TARGET}
	@if [ -d ${HOME}/local/bin/ ]; then\
		strip ${TARGET};\
		install --compare --verbose ${TARGET} ${HOME}/local/bin;\
	fi

.PHONY: clean distclean install
