FROM espressif/idf:release-v4.4

ARG PROXY=""

# apt-get で入力待ちにならないようにするためセット
ENV DEBIAN_FRONTEND=noninteractive
# proxy 設定
ENV HTTP_PROXY=$PROXY
ENV http_proxy=$PROXY
ENV https_proxy=$PROXY
ENV HTTPS_PROXY=$PROXY
RUN if [ "$PROXY" != "" ]; then						\
	echo set $PROXY;						\
	echo "Acquire::http::Proxy \"$PROXY\";" >> /etc/apt/apt.conf;	\
	echo "Acquire::https::Proxy \"$PROXY\";" >> /etc/apt/apt.conf;	\
fi
RUN apt update
RUN apt-get install -y \
	git \
	make \
	diffutils \
	gcc \
	curl

RUN if [ "$PROXY" != "" ]; then			\
    git config --global http.proxy $PROXY;	\
fi
RUN apt update

RUN mkdir -p /proj
WORKDIR /proj
RUN git clone --depth 1 https://github.com/bluekitchen/btstack.git
WORKDIR /proj/btstack/port/esp32
RUN ./integrate_btstack.py
WORKDIR /proj


#ENTRYPOINT ["/opt/esp/entrypoint.sh"]
CMD ["/bin/bash"]
