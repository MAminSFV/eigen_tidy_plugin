# ===============================
# Stage 1: Build environment
# Use official Ubuntu and install LLVM/Clang from apt.llvm.org
# ===============================
FROM ubuntu:22.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG CLANG_VERSION=16

# Base tools and dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      curl \
      gnupg \
      lsb-release \
      # build & packaging tools
      cmake \
      ninja-build \
      dpkg-dev \
      build-essential \
      git \
      # For testing
      libeigen3-dev \
      libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Add the official LLVM APT repository and install requested version
RUN echo "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs)-${CLANG_VERSION} main" | tee /etc/apt/sources.list.d/llvm.list \
    && curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc > /dev/null \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        llvm-${CLANG_VERSION} \
        llvm-${CLANG_VERSION}-dev \
        clang-${CLANG_VERSION} \
        clang-tidy-${CLANG_VERSION} \
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
