# get and configure an image
FROM debian:stretch-slim
RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    software-properties-common \
    build-essential \    
    libssl-dev \
    libiodbc2-dev \
    mysql-server \
    default-libmysqlclient-dev \
    g++-7 \
    wget
    
# RUN add-apt-repository ppa:ubuntu-toolchain-r/test
# RUN apt update
# RUN apt install g++-7 -y
# RUN ls -la /usr/bin/ | grep -oP "[\S]*(gcc|g\+\+)(-[a-z]+)*[\s]" | xargs bash -c 'for link in ${@:1}; do ln -s -f "/usr/bin/${link}-${0}" "/usr/bin/${link}"; done' 7
# get and build wt4
WORKDIR /root
RUN wget https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz --no-check-certificate
RUN tar -zxvf cmake-3.5.2.tar.gz
WORKDIR /root/cmake-3.5.2
RUN ./bootstrap --prefix=/usr
RUN make
RUN make install

WORKDIR /root
RUN wget --no-check-certificate https://pocoproject.org/releases/poco-1.11.1/poco-1.11.1-all.tar.gz
RUN tar -zxvf poco-1.11.1-all.tar.gz
WORKDIR /root/poco-1.11.1-all/cmake-build
RUN cmake .. && \
    cmake --build .
RUN cmake --build . --target install
RUN ldconfig

# build our app
WORKDIR /root/social_net
ADD . /root/social_net
RUN cmake -S . -B build
RUN cmake --build build
RUN cmake --build build --target install
CMD /root/social_net/build/social_net --port $PORT
