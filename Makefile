all: remove clean build run

# Removes docker container if it is running
remove:
	-docker stop stun

# Removes docker image if it is running
clean: 
	@docker rmi stunserver

# Builds the docker image from Dockerfile
build: Dockerfile
	@docker build -t stunserver .

# Runs the recently built image and open the UDP port
run:
	docker run -d -p 3478:3478/udp --rm --name stun stunserver