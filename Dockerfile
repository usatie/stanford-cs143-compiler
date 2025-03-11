FROM --platform=linux/amd64 ubuntu:20.04

# Avoid prompts during installation
ARG DEBIAN_FRONTEND=noninteractive

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
    && rm -rf /var/lib/apt/lists/*

# Create CS143 directory structure
RUN mkdir -p /usr/class/cs143

# Copy the project files
COPY . /usr/class/cs143/

# Set up environment
ENV PATH="/usr/class/cs143/bin:${PATH}"

# Set working directory
WORKDIR /usr/class/cs143

# Optional: Specify a command to run when the container starts
CMD ["/bin/bash"]