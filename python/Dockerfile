FROM ubuntu:16.04

RUN apt-get update
RUN apt-get -y install python
RUN apt-get -y install python-gdal

RUN mkdir -p src/ndvi
WORKDIR /src/ndvi

COPY . .