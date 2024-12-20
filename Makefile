CXX=g++-14
CXXSTD=c++20
CXXFLAGS= -DDEBUG


dev: main.cpp constants.cpp container_minifs.cpp
	$(CXX) $(CXXFLAGS) main.cpp constants.cpp container_minifs.cpp -std="$(CXXSTD)" -o main
	sudo ./main $(ARGS)