all: compute_saliency

compute_saliency: compute_saliency.cpp
	clang++ -framework OpenCL compute_saliency.cpp -o compute_saliency

image.o: image.cpp image.hpp
	clang++ -c $< -o $@