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
    apt-utils make bash quilt rsync tree vim less curl \
    python3 python3-pip git binutils build-essential gdb \
    sed gawk wget bc coreutils zip unzip chrpath \
    ca-certificates locales file iproute2 inetutils-ping \
  && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
  && locale-gen en_US.UTF-8 \
  && ${PKG_CLEANUP}

#COPY "entrypoint.sh" "/bin/entrypoint.sh"
#RUN set -xe && chmod a+x "/bin/entrypoint.sh"
#COPY "bash.bashrc" /etc/

### Entry point
##############################################################################
#ENTRYPOINT ["/bin/entrypoint.sh"]

# Setting the command to launch
CMD ["bash", "-l"]
