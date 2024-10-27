# Docker Image Based on https://github.com/lukstep/raspberry-pi-pico-docker-sdk
FROM ubuntu:24.10 as build

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install --no-install-recommends -y \
                       git \
                       ca-certificates \
                       python3 \
                       tar \
                       build-essential \
                       gcc-arm-none-eabi \
                       libnewlib-arm-none-eabi \
                       libstdc++-arm-none-eabi-newlib \
                       cmake && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Raspberry Pi Pico SDK
ARG SDK_PATH=/usr/local/picosdk
RUN git clone --depth 1 --branch 2.0.0 https://github.com/raspberrypi/pico-sdk $SDK_PATH && \
    cd $SDK_PATH && \
    git submodule update --init

ENV PICO_SDK_PATH=$SDK_PATH

# Build the Project
RUN mkdir /app
WORKDIR /app
COPY . .

RUN cmake -B build -DPICO_BOARD=pico_w -S .
RUN make -C build/

# Separate the Binaries for Exporting
FROM scratch
COPY --from=build /app/build/src/retro_pico_switch.uf2 /