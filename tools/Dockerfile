# ===============================
# Stage 1: Clang/LLVM Base image
# ===============================
FROM ubuntu:22.04 AS clang-base

ARG DEBIAN_FRONTEND=noninteractive
ARG CLANG_VERSION=16

# Base tools for installing LLVM/Clang
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      curl \
      gnupg \
      lsb-release \
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
        cmake \
        ninja-build \
        dpkg-dev \
        build-essential \
        git \
        libeigen3-dev \
        libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Ensure clang and llvm tools are the right version
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} 100 && \
    update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-${CLANG_VERSION} 100


# ===============================
# Stage 2: Builder
# ===============================
FROM clang-base AS builder

ARG CLANG_VERSION=16
ENV DEBIAN_FRONTEND=noninteractive

## Using clang-base directly provides clang/llvm, cmake, ninja, and build deps

# Ensure unversioned symlinks exist in this stage
RUN ln -sf /usr/bin/clang-${CLANG_VERSION} /usr/bin/clang && \
    ln -sf /usr/bin/clang++-${CLANG_VERSION} /usr/bin/clang++ && \
    ln -sf /usr/bin/llvm-config-${CLANG_VERSION} /usr/bin/llvm-config && \
    ln -sf /usr/bin/clang-tidy-${CLANG_VERSION} /usr/bin/clang-tidy

# Build setup
WORKDIR /src
COPY . .

# Build using project release script (uses CMake, CPack, tests)
RUN CC=clang CXX=clang++ tools/release.sh -t Release


# ===============================
# Stage 3: Runtime / Artifacts
# ===============================
FROM debian:stable-slim AS runtime

WORKDIR /opt/eigen-tidy-plugin

# Copy only the built .deb package(s) from builder stage
COPY --from=builder /src/build/*.deb .

# Default command shows artifacts
CMD ["ls", "-lh", "/opt/eigen-tidy-plugin"]
