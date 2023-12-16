#
# Copyright (C) 2020 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Intel Corporation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#


FROM ubuntu:18.04 as sgxbase
RUN apt-get update && apt-get install -y \
    gnupg \
    wget

RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' > /etc/apt/sources.list.d/intel-sgx.list
RUN wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -
RUN apt-get update 

FROM sgxbase as sgx_sample_builder
# App build time dependencies
RUN apt-get install -y build-essential

WORKDIR /opt/intel
RUN wget https://download.01.org/intel-sgx/sgx-linux/2.8/distro/ubuntu18.04-server/sgx_linux_x64_sdk_2.8.100.3.bin
RUN chmod +x sgx_linux_x64_sdk_2.8.100.3.bin
RUN echo 'yes' | ./sgx_linux_x64_sdk_2.8.100.3.bin
WORKDIR /opt/intel/sgxsdk/SampleCode/SampleEnclave
RUN SGX_DEBUG=0 SGX_MODE=HW SGX_PRERELEASE=1 make

#FROM sgxbase as sample
RUN apt-get install -y \
    libcurl4 \
    libprotobuf10 \
    libssl1.1

# No AESM daemon, only AESM client side API support for launch.
# For applications requiring attestation, add libsgx-quote-ex
RUN apt-get install -y --no-install-recommends libsgx-launch libsgx-urts


#CMD ./app

# Installing CMAKE
RUN apt-get update \
  && apt-get -y install build-essential \
  && apt-get install -y wget \
  && rm -rf /var/lib/apt/lists/* \
  && wget https://github.com/Kitware/CMake/releases/download/v3.24.1/cmake-3.24.1-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /opt/cmake-3.24.1 \
      && /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake-3.24.1 \
      && rm /tmp/cmake-install.sh \
      && ln -s /opt/cmake-3.24.1/bin/* /usr/local/bin

# CMAKE installation end

# Rabbit-MQ download
RUN apt-get update \
  && apt-get -y install git -y \
  && apt-get -y install libssl-dev -y

WORKDIR /
RUN git clone https://github.com/alanxz/rabbitmq-c.git 
WORKDIR /rabbitmq-c
RUN mkdir build && cd build
RUN cmake .
RUN cmake --build .

#Copy librabbitmq.so to /usr/lib
RUN cp /rabbitmq-c/librabbitmq/librabbitmq* /usr/lib/
#Copy rabbit mq header to /usr/include
RUN cp -r /rabbitmq-c/include/*  /usr/include/

#installing curl - required for rabbitmq-c
RUN apt-get update && apt-get -y install libcurl4-openssl-dev -y \
&& apt-get -y install zlib1g-dev -y

# rabbitmq C library installation done

# installing homebrew

RUN apt-get update && \
    apt-get install build-essential curl file git ruby-full locales --no-install-recommends -y && \
    rm -rf /var/lib/apt/lists/*

RUN localedef -i en_US -f UTF-8 en_US.UTF-8

RUN useradd -m -s /bin/bash linuxbrew && \
    echo 'linuxbrew ALL=(ALL) NOPASSWD:ALL' >>/etc/sudoers

USER linuxbrew
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/Linuxbrew/install/master/install.sh)"

USER root
ENV PATH="/home/linuxbrew/.linuxbrew/bin:${PATH}"

# This fix brew update error
RUN /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

# brew installation done

#installing rabbitmq server
RUN brew install rabbitmq





#installing NODEJS

ENV NODE_VERSION=16.13.0
RUN apt install -y curl
RUN curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
ENV NVM_DIR=/root/.nvm
RUN . "$NVM_DIR/nvm.sh" && nvm install ${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm use v${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm alias default v${NODE_VERSION}
ENV PATH="/root/.nvm/versions/node/v${NODE_VERSION}/bin/:${PATH}"
RUN node --version
RUN npm --version





## COPY and build SGX-MR-DEV

WORKDIR /demo/data
COPY data .

WORKDIR /demo/lib_sim
#  Intel's SDK docker officilly does not provide simulation mode libraries/ manually copying the pre-installed library
COPY lib_sim .
RUN cp * /usr/lib

# copying SGX-MR dev and installing it
WORKDIR /demo/sgx-mr-dev
COPY sgx-mr-dev .
# RUN SGX_DEBUG=0 SGX_MODE=HW SGX_PRERELEASE=1 make
RUN SGX_DEBUG=0 SGX_MODE=SIM  make
# SGX-MR dev building complete

## Copy and build webserver
WORKDIR /demo/webserver
COPY webserver .
COPY startup.sh .

# COPYing the SGX Binary in Webserver directory
RUN cp ../sgx-mr-dev/enclave.signed.so .

EXPOSE 10000
EXPOSE 5672
EXPOSE 15672

# running rabbitmq server # This command does not work with RUN, needto make a statup file for that
# RUN CONF_ENV_FILE="/home/linuxbrew/.linuxbrew/etc/rabbitmq/rabbitmq-env.conf" /home/linuxbrew/.linuxbrew/opt/rabbitmq/sbin/rabbitmq-server &

# Initializing container startup
CMD ./startup.sh