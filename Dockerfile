FROM ubuntu:latest 

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    clang-tidy \
    clang-format \
    wget \
    unzip



COPY . .

RUN chmod +x ./scripts.sh && ./scripts.sh