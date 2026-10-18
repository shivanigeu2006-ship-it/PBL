# SmartStock build (Linux / macOS / MinGW).  Usage: make        -> builds ./smartstock
#                                                   make test   -> unit tests + API tests
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude -Isrc
LDFLAGS  ?= -pthread
ifeq ($(OS),Windows_NT)
  LDFLAGS += -lws2_32 -lmswsock
  EXE := .exe
endif

all: smartstock$(EXE)

smartstock$(EXE): src/main.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) src/main.cpp $(LDFLAGS) -o $@

test_core$(EXE): tests/test_core.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) tests/test_core.cpp $(LDFLAGS) -o $@

test: test_core$(EXE) smartstock$(EXE)
	./test_core$(EXE)
	@rm -rf /tmp/ss_test; ./smartstock$(EXE) --port 8099 --data /tmp/ss_test & echo $$! > .pid; sleep 1.5; \
	python3 tests/api_test.py 8099; r=$$?; kill `cat .pid`; rm -f .pid; exit $$r

clean:
	rm -f smartstock smartstock.exe test_core test_core.exe .pid

.PHONY: all test clean
