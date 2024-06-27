# Dockerfile to build R2D
# Dr. Stevan Gavrilovic

# Need to build with jammy to ensure compatibility with QGIS plugin versions
FROM ubuntu:jammy

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies
RUN apt-get update && apt-get install -y \
    software-properties-common \
    curl \
    gnupg2 \
    && apt-get install -y bison build-essential ca-certificates ccache cmake wget cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libprotobuf-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pkg-config poppler-utils protobuf-compiler spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb libproj-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Download and install protobuf 3.19.4 - need this specific version
WORKDIR /tmp
RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-all-3.19.4.tar.gz && \
    tar -xvzf protobuf-all-3.19.4.tar.gz && \
    cd protobuf-3.19.4 && \
    ./configure && \
    make && \
    make check && \
    make install && \
    ldconfig


# Install the qt stuff
RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libqt5svg5-dev \
    libqt5webkit5-dev \
    libqt5xmlpatterns5-dev \
    libqt5serialport5-dev \
    libqt5sql5-sqlite \
    libqt5opengl5-dev \
    libqscintilla2-qt5-dev \
    libqwt-qt5-dev \
    libqt5multimedia5 \
    libqt5charts5-dev \
    qtpositioning5-dev \
    libqt5webengine5 \
    libqt5webenginewidgets5 \
    qtwebengine5-dev \
    qt3d-assimpsceneimport-plugin \
    qt3d-defaultgeometryloader-plugin \
    qt3d-gltfsceneio-plugin \
    qt3d-scene2d-plugin \
    qt3d5-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
    

# Set up the Qt environment
RUN echo "source /opt/qt515/bin/qt515-env.sh" >> /etc/profile.d/qt.sh


# Set GDAL environment variables
ENV CPLUS_INCLUDE_PATH=/usr/include/gdal
ENV C_INCLUDE_PATH=/usr/include/gdal

 
# Clean up
RUN rm -rf /tmp/protobuf-3.19.4*

# Clone required repositories
# Frank TODO: fix simcenter common and R2D diffs
RUN git clone --branch master https://github.com/NHERI-SimCenter/SimCenterCommon.git && cd SimCenterCommon && git checkout cbbd0c6
RUN git clone --branch main --depth 1 https://github.com/NHERI-SimCenter/QGISPlugin.git
RUN git clone --branch master --depth 1 https://github.com/NHERI-SimCenter/R2DTool


# Set working directory
WORKDIR ./R2DTool

# Libzip was renamed to .4 though functionally same as .5
RUN ln -s /usr/lib/x86_64-linux-gnu/libzip.so.4 /usr/lib/x86_64-linux-gnu/libzip.so.5 && ldconfig

# Build project
RUN /bin/bash -c "export CPLUS_INCLUDE_PATH=/usr/include/x86_64-linux-gnu/qt5:$CPLUS_INCLUDE_PATH && mkdir build && cd build && qmake ../R2D.pro && make -j 12 && rm -fr .obj *.o *.cpp"


# Install python 3.9
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository ppa:deadsnakes/ppa \
    && apt-get update \
    && apt-get install -y \
        python3.9 \
        python3.9-venv \
        python3.9-dev \
        python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && pip3 install conan==1.60.1 


# Build SimCenter Backend Applications
#    note: need newer gcc, gcc-10 which necessitates removing old conan

RUN git clone https://github.com/NHERI-SimCenter/SimCenterBackendApplications.git \
    && cp ./SimCenterBackendApplications/modules/performUQ/SimCenterUQ/nataf_gsa/CMakeLists.txt.UBUNTU ./SimCenterBackendApplications/modules/performUQ/SimCenterUQ/nataf_gsa/CMakeLists.txt \
    && rm -fr ~/.conan \
    && apt-get update \
    && apt-get install -y liblapack-dev libomp-dev libssl-dev apt-transport-https \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get install -y gcc-10 g++-10 gfortran-10 \
    && export CC=gcc-10 \
    && export CXX=g++-10 \
    && export FC=gfortran-10 \
    && cd SimCenterBackendApplications \
    && mkdir build \
    && cd build \
    && conan install .. --build missing \
    && cmake .. \
    && cmake --build . --config Release \
    && cmake --install . \
    && cd ../..


#
# add following for the missing lib libQt5Core.so error that is related to running on some versions linux
# with an older kernel, seemingly need kernel >= 3.5 on host .. soln from Sal T. found in an AskUbuntu thread
#
# may not need this for jammy...
#RUN strip --remove-section=.note.ABI-tag /opt/qt515/lib/libQt5Core.so.5

#
# Finally add a new user simcenter as root cannot run the Qt app
#

RUN useradd -ms /bin/bash simcenter

USER simcenter
