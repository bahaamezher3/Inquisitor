FROM alpine:latest

# Install build dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    libpcap-dev \
    tcpdump \
    net-tools \
    iproute2 \
    vim \
    gdb \
    git \
    make \
    g++ \
    linux-headers \
    bash

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the project
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make

# Set the entry point
ENTRYPOINT ["/bin/sh"]