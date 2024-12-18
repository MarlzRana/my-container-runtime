CC=g++-14
CPLUSPLUSSTD=c++20

dev: main.cpp constants.cpp container_minifs.cpp
	$(CC) main.cpp constants.cpp container_minifs.cpp -std="$(CPLUSPLUSSTD)" -o main
	sudo ./main