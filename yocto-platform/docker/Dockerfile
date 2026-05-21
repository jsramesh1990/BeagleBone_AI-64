FROM ubuntu:22.04

# Install Yocto dependencies
RUN apt-get update && apt-get install -y \
    gawk \
    wget \
    git-core \
    diffstat \
    unzip \
    texinfo \
    gcc \
    build-essential \
    chrpath \
    socat \
    cpio \
    python3 \
    python3-pip \
    python3-pexpect \
    xz-utils \
    debianutils \
    iputils-ping \
    libsdl1.2-dev \
    xterm \
    make \
    zstd \
    liblz4-tool \
    curl \
    sudo \
    file \
    locales \
    && rm -rf /var/lib/apt/lists/*

# Install kas
RUN pip3 install kas

# Set locale
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Create build user
RUN useradd -m -s /bin/bash yocto && \
    echo "yocto ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER yocto
WORKDIR /home/yocto

# Entrypoint
COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
