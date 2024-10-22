all: build run

build:
	gcc pubsubs.c -o pubsubs -Ofast

run:
	./pubsubs 9999

install:
	sudo cp pubsubs /usr/local/bin/