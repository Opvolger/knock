FROM debian:trixie AS build

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

FROM scratch

ARG TARGETOS
ARG TARGETARCH
ARG TARGETVARIANT

ARG VERSION=0.8.1

COPY --from=build /build/output/usr/local/knock-${VERSION}-${TARGETARCH}${TARGETVARIANT}.tar.gz /
