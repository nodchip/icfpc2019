CXXFLAGS=-O2 -Wall -Wno-sign-compare -std=c++14
CXXFLAGS+=-I .
CXXFLAGS+=-pg

LDFLAGS=

SRCS=base.cpp game.cpp map_parse.cpp

BUILD_PATH=build
OBJS=$(SRCS:%.cpp=$(BUILD_PATH)/%.o)

# TEST_SRCS=$(LIB_PATH)/googletest/googletest/src/gtest-all.cc
# TEST_OBJS=$(TEST_SRCS:%.cpp=$(BUILD_PATH)/%.o)

.PHONY: all
all: dirs solver

solver: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -Wall $^ -o $@ $(LDFLAGS)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_PATH)

$(BUILD_PATH)/%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS)
