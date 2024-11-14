FROM tangramflex/pro-builder:latest

USER root

RUN mkdir /component && chown 1001 /component

USER 1001:1001

WORKDIR /component

RUN pwd

COPY code-gen/ code-gen/
RUN cd code-gen && make clean && make -j && make install

RUN pwd

COPY Ethan/src/main.cpp /component/src/main.cpp
COPY Ethan/Makefile /component/Makefile

RUN pwd

ENV LD_LIBRARY_PATH=/component/code-gen/local_install/lib

RUN make -j

ENTRYPOINT ["./ethan"]

