CXX=g++-14
CXXSTD=c++20
CXXFLAGS= -DDEBUG -Wall


dev: main.cpp constants.cpp container_overlayfs.cpp containerization.cpp
	$(CXX) $(CXXFLAGS) main.cpp constants.cpp container_overlayfs.cpp containerization.cpp -std="$(CXXSTD)" -o main
	sudo ./main $(ARGS)


dev-valgrind: main.cpp constants.cpp container_overlayfs.cpp
	$(CXX) $(CXXFLAGS) main.cpp constants.cpp container_overlayfs.cpp containerization.cpp -std="$(CXXSTD)" -o main
	sudo valgrind --leak-check=full -s ./main $(ARGS)