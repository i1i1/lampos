FROM debian:9.1
RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install -y \
	gcc-multilib \
	libc6-dev-i386 \
	make \
	curl \
	gawk \
	genisoimage
COPY . /src
RUN groupadd -r lampos && useradd --no-log-init -r -g lampos lampos
RUN mkdir /build
RUN printf "#!/bin/sh\n\
chown lampos:lampos /build /out\n\
runuser -u lampos -- make -C/src BUILD=/build ISO=/out/boot.iso\n" > /entrypoint && \
	chmod +x /entrypoint
ENTRYPOINT [ "/entrypoint" ]
