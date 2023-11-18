CXX = g++
DEBUG_CXXFLAGS = -g -O1 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wall -std=c++17
RELEASE_CXXFLAGS = -O2 -std=c++17

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: search-server

release: CXXFLAGS = $(RELEASE_CXXFLAGS)
release: search-server

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

all: search-server

search-server: $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o output/$@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) output/search-server