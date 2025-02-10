ARG BUILD_TYPE=Debug

FROM --platform=${BUILDPLATFORM} alpine:3.21 AS drogon

RUN apk update && apk add --no-cache \
    build-base \
    util-linux-dev \
    cmake \
    make \
    ninja \
    gcc \
    g++ \
    git \
    postgresql-dev \
    jsoncpp-dev \
    zlib-dev

RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake -GNinja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \
    ninja && ninja install

FROM --platform=${BUILDPLATFORM} alpine:3.21 AS final

RUN apk update && apk add --no-cache --virtual .build-deps \
    build-base \
    cmake \
    make \
    ninja \
    gcc \
    g++ && \
    apk add --no-cache \
    postgresql-dev \
    jsoncpp-dev \
    zlib-dev \
    util-linux-dev \
    libjpeg-turbo-dev \
    libpng-dev \
    tiff-dev \
    opencv-dev \
    libsodium-dev \
    curl \

COPY --from=drogon /usr/local/lib/cmake/Drogon /usr/local/lib/cmake/Drogon
COPY --from=drogon /usr/local/include/drogon /usr/local/include/drogon
COPY --from=drogon /usr/local/lib/libdrogon.a /usr/local/lib/libdrogon.a
COPY --from=drogon /usr/local/include/trantor /usr/local/include/trantor
COPY --from=drogon /usr/local/lib/libtrantor.a /usr/local/lib/libtrantor.a
COPY --from=drogon /usr/local/lib/cmake/Trantor /usr/local/lib/cmake/Trantor

COPY ./src /src
COPY ./include /include
COPY ./CMakeLists.txt /CMakeLists.txt

WORKDIR /api

RUN mkdir build && \
    cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \
    ninja

RUN apk del .build-deps

EXPOSE 8080

CMD ["./build/nanoenv"]