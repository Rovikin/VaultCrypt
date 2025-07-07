CXX = clang++
CXXFLAGS = -O2 -std=c++17
LDFLAGS = -lsodium

all: secure_seed

secure_seed: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f secure_seed
