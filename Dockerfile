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
    bison \
    flex \
    && rm -rf /var/lib/apt/lists/*


# Set the location where vcpkg will be installed
ENV VCPKG_ROOT=/opt/vcpkg

# Download and install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && cd $VCPKG_ROOT \
    && ./bootstrap-vcpkg.sh -disableMetrics

# Establish the working directory
WORKDIR /app

# Install C++ dependencies before copying the source code
COPY vcpkg.json ./
RUN $VCPKG_ROOT/vcpkg install --triplet x64-linux

# Copy the rest of TradeVault
COPY . .

# Configure CMake using the previously installed dependencies
RUN cmake --preset release -DVCPKG_INSTALLED_DIR=/app/vcpkg_installed

# Compile TradeVault
RUN cmake --build --preset release


# Stage 2: Runtime environment
FROM ubuntu:24.04 AS runtime

# Install only the libraries needed to run TradeVault
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*


# Establish the application's working directory
WORKDIR /app

# Copy the compiled executable from the builder stage
COPY --from=builder /app/build/release/src/tradevault_api ./tradevault_api


# Copy application configuration
COPY --from=builder /app/config ./config

# Override the default configuration for Docker
COPY --from=builder /app/config/config.docker.json ./config/config.json

# Copy Swagger and OpenAPI files
COPY --from=builder /app/public ./public


# Create a non-root user to run TradeVault
RUN useradd --system --no-create-home --shell /usr/sbin/nologin tradevault
USER tradevault

# Document the port used by our API
EXPOSE 8080

# Start TradeVault when the container launches
CMD ["./tradevault_api"]
