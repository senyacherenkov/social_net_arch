# get and configure an image
# FROM debian:stretch-slim
FROM ubuntu:trusty
RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    software-properties-common \
    build-essential \    
    libssl-dev \
    libiodbc2-dev \
    mysql-server-5.5 \
    libmysqlclient-dev \
    wget

# g++-7 installation
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN sudo apt update
RUN apt install g++-7 -y
RUN ls -la /usr/bin/ | grep -oP "[\S]*(gcc|g\+\+)(-[a-z]+)*[\s]" | xargs bash -c 'for link in ${@:1}; do ln -s -f "/usr/bin/${link}-${0}" "/usr/bin/${link}"; done' 7

#cmake
WORKDIR /root
RUN wget https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz --no-check-certificate
RUN tar -zxvf cmake-3.5.2.tar.gz
WORKDIR /root/cmake-3.5.2
RUN ./bootstrap --prefix=/usr
RUN make
RUN make install

# poco
WORKDIR /root
RUN wget --no-check-certificate https://pocoproject.org/releases/poco-1.11.1/poco-1.11.1-all.tar.gz
RUN tar -zxvf poco-1.11.1-all.tar.gz
WORKDIR /root/poco-1.11.1-all/cmake-build
RUN cmake .. && \
    cmake --build .
RUN cmake --build . --target install
RUN ldconfig
# build our app
WORKDIR /root/socialnetotus
ADD . /root/socialnetotus

# create db user and tables via mysql file
#solution for Can't connect to local MySQL server through socket '/var/run/mysqld/mysqld.sock'
# ENV MYSQL_ROOT_PASSWORD=1
# RUN touch /var/run/mysqld/mysqld.sock && \
#     touch /var/run/mysqld/mysqld.pid && \
#     chown -R mysql:mysql /var/run/mysqld/mysqld.sock && \
#     chown -R mysql:mysql /var/run/mysqld/mysqld.pid && \
#     chmod -R 644 /var/run/mysqld/mysqld.sock && \
#     chown -R mysql:mysql /var/lib/mysql /var/run/mysqld && \
# RUN mkfifo /var/run/mysqld/mysqld.sock && \
#     chown -R mysql /var/run/mysqld && \
#     service mysql restart && \
#     mysqladmin --user=root password "1" && \
#     echo ${MYSQL_ROOT_PASSWORD} && \
    # mysqladmin -u root -h password '1' && \
# RUN chown -R mysql:mysql /var/lib/mysql && \
#     service mysql start && \
#     mysqladmin --user=root password "1" && \
    # mysql -p1 < poco_server.sql
    # mysqlcheck --check-upgrade --all-databases --auto-repair -u root --password=1 && \
    # mysql_upgrade --force -u root --password=1

WORKDIR /root/socialnetotus/build-dir
RUN cmake .. && \
    cmake --build .
# RUN ls -la

# create the final image
# FROM ubuntu:trusty
# RUN apt-get update
# WORKDIR /root
# COPY --from=builder /root/socialnetotus/build-dir/socialnetotus /root/
WORKDIR /root/socialnetotus
RUN chmod +x start_server.sh
CMD ./start_server.sh $PORT $ADDRESS

