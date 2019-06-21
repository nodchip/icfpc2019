LIB_PATH=../libs
CXXFLAGS=-O2 -Wall -Wno-sign-compare -std=c++14
CXXFLAGS+=-I .
CXXFLAGS+=-pg

LDFLAGS=

SRCS=base.cpp game.cpp map_parse.cpp

BUILD_PATH=build
OBJS=$(SRCS:%.cpp=$(BUILD_PATH)/%.o)

TEST_BUILD_PATH=build
TEST_CXXFLAGS=$(CXXFLAGS) \
	-I$(LIB_PATH)/googletest/googletest \
	-I$(LIB_PATH)/googletest/googletest/include
TEST_LDFLAGS=$(LDFLAGS) \
	-lpthread
TEST_SRCS=$(LIB_PATH)/googletest/googletest/src/gtest-all.cc
TEST_SRCS+=$(wildcard tests/*.cpp)
TEST_OBJS=$(TEST_SRCS:%.cpp=$(BUILD_PATH)/%.o)

TARGETS=solver test
.PHONY: all
all: dirs $(TARGETS)

solver: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -Wall $^ -o $@ $(LDFLAGS)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_PATH) $(TEST_BUILD_PATH)

$(BUILD_PATH)/%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: test
test: $(TEST_OBJS)
	$(CXX) $(TEST_CXXFLAGS) -Wall $^ -o $@ $(TEST_LDFLAGS)

$(BUILD_PATH)/%.o: %.cpp
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@