FROM ubuntu
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install git clang cmake make -y
RUN git clone https://github.com/google/googletest.git --depth 1 --branch main /gtest \
    && mkdir /gtest/build
WORKDIR /gtest/build
RUN cmake .. -DBUILD_GMOCK=OFF && make && cmake --install .
WORKDIR /
RUN rm -rf /gtest && apt-get remove git -y && apt-get clean -y
RUN mkdir /project /build /sampleproj
WORKDIR /build
COPY ./sample_user_project/CMakeLists.txt /sampleproj/CMakeLists.txt
COPY ./sample_user_project/main.cpp /sampleproj/main.cpp
COPY ./test.sh /test.sh
CMD ["sh", "/test.sh"]
