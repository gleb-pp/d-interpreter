FROM alpine
RUN apk add git clang cmake make ncurses
RUN git clone https://github.com/google/googletest.git --depth 1 --branch main /gtest \
    && mkdir /gtest/build
WORKDIR /gtest/build
RUN cmake .. -DBUILD_GMOCK=OFF && make && cmake --install .
WORKDIR /
RUN rm -rf /gtest && apk del git && apk cache purge
RUN mkdir /project /build
WORKDIR /build
COPY ./test.sh /test.sh
CMD ["sh", "/test.sh"]
