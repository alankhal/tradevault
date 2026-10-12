# Stage 1: Build TradeVault
FROM ubuntu:24.04 AS builder

# Install tools needed to compile our C++ application
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ca-certificates \
    python3 \
    && rm -rf /var/lib/apt/lists/*


# Set the location where vcpkg will be installed
ENV VCPKG_ROOT=/opt/vcpkg

# Download and install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && cd $VCPKG_ROOT \
    && ./bootstrap-vcpkg.sh -disableMetrics


# Set the working directory inside the container
WORKDIR /app  #Create/selects apps as the working directory inside the image. Subsequenct instructions execute relative to this location

# Copy TradeVault source files into the container
COPY . .  #First . is the project directory in Windows, the second . is the /app inside the image, additional files won't be added thanks to .dockerignore

# Configure TradeVault using our existing release preset
RUN cmake --preset release   #Uses the current CMakePresets.json to make the linux build 

#Compilation instructions 
RUN cmake --build --preset release
