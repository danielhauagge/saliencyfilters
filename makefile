BIN:=compute_slic compute_saliency test_image test_opencl
OBJS:=image.o opencl.o slicsuperpixels.o utils.o saliencyfilter.o

all: ${BIN}

%.o: %.cpp %.hpp
	clang++ -g -c $< -o $@

compute_slic: compute_slic.cpp ${OBJS}
	clang++ -g ${OBJS} -lfreeimage -framework OpenCL $< -o $@

compute_saliency: compute_saliency.cpp ${OBJS}
	clang++ -g ${OBJS} -lfreeimage -framework OpenCL $< -o $@

test_image: test_image.cpp ${OBJS}
	clang++ -g ${OBJS} -lfreeimage -framework OpenCL $< -o $@

test_opencl: test_opencl.cpp ${OBJS}
	clang++ -g ${OBJS} -lfreeimage -framework OpenCL $< -o $@


clean:
	rm -rf ${OBJS} ${BIN}
