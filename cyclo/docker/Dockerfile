FROM debian:11.0-slim

### Labelling
##############################################################################
LABEL "description"="Debian docker image for building AVR application"
LABEL "author"="software@arreckx.com"

### Create a macro for cleaning the environment and keep the image small
##############################################################################
ENV PKG_CLEANUP "set -xe && apt-get autoclean && apt-get clean && rm -rf /var/lib/apt/lists/*"

### Build
##############################################################################

# Cleanup first
RUN set -xe && apt-get update && apt-get -y upgrade && ${PKG_CLEANUP}

# Install packages
RUN set -xe \
  && apt-get update \
  && apt-get -y install --no-install-recommends \
    apt-utils make bash quilt rsync tree vim less \
    python3 python3-pip git binutils build-essential gdb \
    sed gawk wget bc coreutils chrpath libx11-dev \
    ca-certificates locales file iproute2 inetutils-ping \
    python3-pil \
  && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
  && locale-gen en_US.UTF-8 \
  && ${PKG_CLEANUP}

# Install the avr toolchain
RUN set -xe \
  && wget https://github.com/ZakKemble/avr-gcc-build/releases/download/v11.1.0-1/avr-gcc-11.1.0-x64-linux.tar.bz2 -O /tmp/avr-gcc.tar.tbz \
  && cd /opt \
  && tar xjvf /tmp/avr-gcc.tar.tbz \
  && rm -f /tmp/avr-gcc.tar.xz \
  && cd /opt \
  && ln -s /opt/avr-gcc-* /opt/avr-gcc \
  && echo 'PATH=$PATH:/opt/avr-gcc/bin' >> /etc/bash.bashrc

# Add the path for all processes
ENV PATH='PATH=/usr/sbin:/usr/bin:/sbin:/bin:/opt/avr-gcc/bin'

# Set the prompt
RUN set -xe \
  && echo "PS1='\n\[\e[0;36m\]\T \[\e[1;30m\][\H\[\e[1;30m\]:\[\e[0;37m\]o \[\e[0;32m\]+4\[\e[1;30m\]] \[\e[1;37m\]\w\[\e[0;37m\] \n$ '" >> /etc/bash.bashrc

# Setting the command to launch
CMD ["bash", "-l"]
