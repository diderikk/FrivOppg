all: clean build remove run

# Removes docker image if it is running
clean: 
	-docker rmi stunserver

# Builds the docker image from Dockerfile
build: Dockerfile
	@docker build -t stunserver .

# Removes docker container if it is running
remove:
	-docker stop stun

# Runs the recently built image and open the UDP port
run:
	docker run -d -p 8080:8080/udp --rm --name stun stunserver