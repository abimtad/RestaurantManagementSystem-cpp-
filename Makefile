CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS :=

SRCS := $(wildcard src/*.cpp)
APP_SRCS := $(filter-out src/tests.cpp, $(SRCS))
TEST_SRCS := $(filter-out src/main.cpp, $(SRCS))
OUT := restaurant
TEST_OUT := restaurant_tests

.PHONY: all clean run

all: $(OUT)

$(OUT): $(APP_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_OUT): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(OUT)
	./$(OUT)

test: $(TEST_OUT)
	./$(TEST_OUT)

clean:
	rm -f $(OUT) $(TEST_OUT)
