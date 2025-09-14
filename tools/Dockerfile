# ===============================
# Stage 1: Build environment
# ===============================
ARG CLANG_VERSION=16
FROM clang:${CLANG_VERSION} AS builder

# Set the same ARG inside this stage
ARG CLANG_VERSION

# Install essential build and packaging tools + matching LLVM
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    dpkg-dev \
    build-essential \
    git \
    # LLVM core + dev headers + clang-tidy
    llvm-${CLANG_VERSION} \
    llvm-${CLANG_VERSION}-dev \
    clang-tidy-${CLANG_VERSION} \
    # For testing
    libeigen3-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*


# Ensure clang and llvm tools are the right version
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} 100 && \
    update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-${CLANG_VERSION} 100

# Build setup
WORKDIR /src
COPY . .

# Configure & build project with CMake + Clang
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++
RUN cmake --build build --target package

# ===============================
# Stage 2: Runtime / Artifacts
# ===============================
FROM debian:stable-slim

WORKDIR /opt/mypackage

# Copy only the built .deb package(s)
COPY --from=builder /src/build/*.deb .

# Default command shows artifacts
CMD ["ls", "-lh", "/opt/mypackage"]
