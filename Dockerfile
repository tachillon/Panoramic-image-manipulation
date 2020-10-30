FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive
# Core Linux Deps
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y --fix-missing --no-install-recommends apt-utils \
    build-essential                  \
    curl                             \
    binutils                         \
    gdb                              \
    git                              \
    freeglut3                        \
    freeglut3-dev                    \
    libxi-dev                        \
    libxmu-dev                       \
    gfortran                         \
    pkg-config                       \
    python-numpy                     \
    python-dev                       \
    python-setuptools                \
    libboost-python-dev              \
    libboost-thread-dev              \
    pbzip2                           \
    rsync                            \
    software-properties-common       \
    libboost-all-dev                 \
    libopenblas-dev                  \
    libtbb2                          \
    libtbb-dev                       \
    libjpeg-dev                      \
    libpng-dev                       \
    libtiff-dev                      \
    libgraphicsmagick1-dev           \
    libavresample-dev                \
    libavformat-dev                  \
    libhdf5-dev                      \
    libpq-dev                        \
    libgraphicsmagick1-dev           \
    libavcodec-dev                   \
    libgtk2.0-dev                    \
    liblapack-dev                    \
    liblapacke-dev                   \
    libswscale-dev                   \
    libcanberra-gtk-module           \
    libboost-dev                     \
    libboost-all-dev                 \
    libeigen3-dev                    \
    wget                             \
    vim                              \
    qt5-default                      \
    unzip                            \
    zip                              \
    ffmpeg                           \
    libv4l-dev                       \
    libatlas-base-dev                \
    libgphoto2-dev                   \
    libgstreamer-plugins-base1.0-dev \
    libdc1394-22-dev                 \
                                  && \
    apt-get clean                 && \
    rm -rf /var/lib/apt/lists/*   && \
    apt-get clean && rm -rf /tmp/* /var/tmp/*
ENV DEBIAN_FRONTEND noninteractive

# Install cmake version that supports anaconda python path
RUN wget -O cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.15.4/cmake-3.15.4-Linux-x86_64.tar.gz && \
    tar -xvf cmake.tar.gz            && \
    cd /cmake-3.15.4-Linux-x86_64    && \
    cp -r bin /usr/                  && \
    cp -r share /usr/                && \
    cp -r doc /usr/share/            && \
    cp -r man /usr/share/            && \
    cd /                             && \
    rm -rf cmake-3.15.4-Linux-x86_64 && \
    rm -rf cmake.tar.gz

ARG PYTHON=python3
ARG PIP=pip3

# See http://bugs.python.org/issue19846
ENV LANG C.UTF-8

RUN apt-get update && apt-get install -y \
    ${PYTHON} \
    ${PYTHON}-pip

RUN ${PIP} install --upgrade pip

RUN ${PIP} install --upgrade \
    setuptools    \
    hdf5storage   \
    h5py          \
    py3nvml       \
    opencv-python \
    scikit-image  \
    scikit-learn  \
    matplotlib    \
    pyinstrument

ARG USE_CUDA=OFF
                                                               
RUN git clone --branch 4.5.0 --depth 1 https://github.com/opencv/opencv.git  && \
    cd opencv && mkdir build && cd build                                     && \
    cmake ..                                \
          -DBUILD_TESTS=OFF                 \
          -DBUILD_PERF_TESTS=OFF            \
          -DCMAKE_BUILD_TYPE=RELEASE        \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DOPENCV_ENABLE_NONFREE=ON        \
          -DBUILD_EXAMPLES=OFF           && \
    make -j $(nproc)       && \
    make install           && \
    ldconfig               && \
    rm -rf /opencv         && \
    rm -rf /opencv_contrib

COPY ./CMakeLists.txt /opt/CMakeLists.txt
COPY ./src /opt/src

RUN cd /opt                       && \
    mkdir build                   && \
    cd build                      && \
    cmake ..                         \
          -DWITH_CUDA=${USE_CUDA} && \
    make -j $(nproc)              && \
    make install                  && \
    ldconfig