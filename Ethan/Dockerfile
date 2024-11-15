FROM tangramflex/pro-builder:latest

USER root

RUN mkdir /component && chown 1001 /component

USER 1001:1001

WORKDIR /component

RUN pwd

#COPY Ethan/code-gen/ /component/code-gen/
COPY code-gen/ /component/code-gen/
RUN cd code-gen && make clean && make -j && make install

RUN pwd

RUN cd ..

COPY Ethan/src/main.cpp /component/src/main.cpp
COPY Ethan/Makefile /component/Makefile
COPY Ethan/build.sh /component/build.sh

RUN pwd

RUN make 

ENV LD_LIBRARY_PATH=/component/code-gen/build/libs:/component/code-gen/local_install/lib

RUN make 

ENTRYPOINT ["./ethan"]

