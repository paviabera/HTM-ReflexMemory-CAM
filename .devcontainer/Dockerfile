# [Choice] Debian version (use bullseye on local arm64/Apple Silicon): bullseye, buster
FROM mcr.microsoft.com/vscode/devcontainers/base:bookworm

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update && apt upgrade -y \
    && apt install -y --no-install-recommends software-properties-common \
    build-essential curl git libffi-dev libsqlite3-dev libbz2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/

ENV LD_LIBRARY_PATH=/usr/local/lib

RUN wget "https://www.python.org/ftp/python/3.13.3/Python-3.13.3.tgz" \
    && tar xzvf Python-3.13.3.tgz \
    && cd Python-3.13.3 \
    && ./configure --enable-shared --enable-loadable-sqlite-extensions --enable-optimizations \
    && make -j4 \
    && make install \
    && cd /usr/src/ \
    && wget https://bootstrap.pypa.io/pip/get-pip.py \
    && python3.13 get-pip.py \
    && rm -rfv get-pip.py Python-3.13.3.tgz Python-3.13.3 \
    && update-alternatives --install /usr/bin/python3 python3 /usr/local/bin/python3.13 1

RUN git clone "https://github.com/htm-community/htm.core" \
    && cd htm.core \
    && git reset --hard 235fe063cfb4a9c59a9a5eb1fd8dd0d2d618351b \
    && sed -i '7d' requirements.txt \
    && sed -i '5d' requirements.txt \
    && echo numpy >> requirements.txt \
    && echo cmake==3.31.6 >> requirements.txt \
    && python3.13 -m ensurepip --upgrade \
    && python3.13 -m pip install setuptools packaging \
    && python3.13 -m pip install -r requirements.txt \
    && python3.13 setup.py install \
    && cd /usr/src/ \
    && rm -rfv htm.core

COPY requirements.txt ./

RUN python3.13 -m pip install -r requirements.txt
