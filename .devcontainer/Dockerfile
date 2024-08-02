# [Choice] Debian version (use bullseye on local arm64/Apple Silicon): bullseye, buster
ARG VARIANT=bookworm
FROM mcr.microsoft.com/vscode/devcontainers/base:${VARIANT}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update && apt upgrade -y \
    && apt install -y --no-install-recommends software-properties-common \
    build-essential curl git libffi-dev libsqlite3-dev libbz2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/

ENV LD_LIBRARY_PATH=/usr/local/lib

RUN wget "https://www.python.org/ftp/python/3.11.9/Python-3.11.9.tgz" \
    && tar xzvf Python-3.11.9.tgz \
    && cd Python-3.11.9 \
    && ./configure --enable-shared --enable-loadable-sqlite-extensions --enable-optimizations \
    && make -j4 \
    && make install \
    && cd /usr/src/ \
    && wget https://bootstrap.pypa.io/pip/get-pip.py \
    && python3.11 get-pip.py \
    && rm -rfv get-pip.py Python-3.11.9.tgz Python-3.11.9

RUN git clone "https://github.com/htm-community/htm.core" \
    && python3.11 -m ensurepip --upgrade \
    && python3.11 -m pip install setuptools packaging \
    && python3.11 -m pip install --no-cache-dir -r htm.core/requirements.txt \
    && cd htm.core \
    && python3.11 setup.py install \
    && rm -rfv /usr/src/htm.core

COPY requirements.txt ./

RUN python3.11 -m pip install --no-cache-dir -r requirements.txt
    