FROM eclipse-temurin:8u312-b07-jre-focal AS final-amd64
FROM eclipse-temurin:8u312-b07-jre-focal AS final-arm64
FROM eclipse-temurin:8u312-b07-jre-focal AS final-armv7
FROM eclipse-temurin:25-jre-noble AS final-riscv64

FROM final-$TARGETARCH$TARGETVARIANT AS build

# hook into docker BuildKit --platform support
# see https://docs.docker.com/engine/reference/builder/#automatic-platform-args-in-the-global-scope
ARG TARGETOS
ARG TARGETARCH
ARG TARGETVARIANT

# Version in tar.gz file
ARG VERSION=0.8.1

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt install -y  libpcap-dev \
                    autoconf \
                    dh-autoreconf \
                    make

RUN mkdir -p /build/output/usr/local
WORKDIR /build

COPY . .

RUN autoreconf -fi && \
    ./configure --prefix=/build/output/usr/local && \
    make && make install

WORKDIR /build/output/usr/local

RUN tar -czf knock-${VERSION}-${TARGETARCH}${TARGETVARIANT}.tar.gz *

# musl build for the alpine based images (only amd64)
FROM alpine:3.21 AS build-alpine

ARG VERSION=0.8.1

RUN apk add --no-cache libpcap-dev \
                       autoconf \
                       automake \
                       build-base

RUN mkdir -p /build/output/usr/local
WORKDIR /build

COPY . .

RUN autoreconf -fi && \
    ./configure --prefix=/build/output/usr/local && \
    make && make install

WORKDIR /build/output/usr/local

RUN tar -czf knock-${VERSION}-alpine-amd64.tar.gz *

FROM scratch AS export

ARG TARGETOS
ARG TARGETARCH
ARG TARGETVARIANT

ARG VERSION=0.8.1

COPY --from=build /build/output/usr/local/knock-${VERSION}-${TARGETARCH}${TARGETVARIANT}.tar.gz /

FROM export AS export-amd64
ARG VERSION=0.8.1
COPY --from=build-alpine /build/output/usr/local/knock-${VERSION}-alpine-amd64.tar.gz /

FROM export AS export-arm64
FROM export AS export-armv7
FROM export AS export-riscv64

FROM export-$TARGETARCH$TARGETVARIANT
