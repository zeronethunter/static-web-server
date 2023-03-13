FROM ubuntu:latest

RUN apt -y update && apt -y install \
    cmake \
    g++ \
    libevent-dev \
    make \
    gcc

WORKDIR /server

COPY / .

RUN mkdir build && cd build && cmake .. && make

RUN touch /etc/httpd.conf
RUN mkdir -p /var/www/html

RUN chmod +x ./build/static_web_server

EXPOSE 80

CMD ["./build/static_web_server"]
