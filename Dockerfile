FROM ubuntu:20.04

# Avoid prompts during installation
ARG DEBIAN_FRONTEND=noninteractive

# Note: Build with platform flag instead:
# docker build --platform linux/amd64 -t cs143-compiler .

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    flex \
    bison \
    csh \
    libxaw7-dev \
    gcc-multilib \
    git \
    gdb \
    nano \
    vim \
	wget \
    && rm -rf /var/lib/apt/lists/*

# Create CS143 directory structure
RUN mkdir -p /usr/class/cs143

# Set up environment
ENV PATH="/usr/class/cs143/bin:${PATH}"

# Set working directory
WORKDIR /usr/class/cs143

# Note: Instead of COPY, use a volume mount when running the container:
# docker run -v /path/on/host:/usr/class/cs143 -it image_name
CMD ["/bin/bash"]
