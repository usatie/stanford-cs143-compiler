.PHONY: build run clean

IMAGE_NAME=cs143-compiler

build:
	docker build --platform linux/amd64 -t $(IMAGE_NAME) .

run:
	docker run --platform linux/amd64 -v "$(shell pwd)":/usr/class/cs143 --rm -it $(IMAGE_NAME) 

clean:
	docker rmi $(IMAGE_NAME)
