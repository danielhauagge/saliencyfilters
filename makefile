BIN:=compute_slic compute_saliency test_image test_opencl
OBJS:=image.o opencl.o slicsuperpixels.o utils.o saliencyfilter.o colorconversion.o

CFLAGS=-g
LDFLAGS=-lfreeimage -framework OpenCL -framework CMDCore -lboost_system

all: ${BIN}

%.o: %.cpp %.hpp
	clang++ ${CFLAGS} -c $< -o $@

compute_slic: compute_slic.cpp ${OBJS} *.hpp
	clang++ ${CFLAGS} ${OBJS} ${LDFLAGS} $< -o $@

compute_saliency: compute_saliency.cpp ${OBJS}  *.hpp
	clang++ ${CFLAGS} ${OBJS} ${LDFLAGS} $< -o $@

test_image: test_image.cpp ${OBJS}  *.hpp
	clang++ ${CFLAGS} ${OBJS} ${LDFLAGS} $< -o $@

test_opencl: test_opencl.cpp ${OBJS}  *.hpp
	clang++ ${CFLAGS} ${OBJS} ${LDFLAGS} $< -o $@

test:
	./compute_saliency test.jpg saliency.pgm
	mogrify -format png saliency.pgm
	open saliency.png

clean:
	rm -rf ${OBJS} ${BIN}
