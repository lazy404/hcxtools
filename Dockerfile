FROM debian:9

RUN apt-get update
RUN apt-get install -y aptitude libpcap-dev libssl-dev libcurl4-openssl-dev make gcc libz-dev
WORKDIR /hcxtools
ADD ./ .
RUN make && make install
