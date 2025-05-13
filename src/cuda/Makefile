CXX=g++
NVCC=nvcc
ARCH=-gencode=arch=compute_52,code=sm_52 \
	-gencode=arch=compute_60,code=sm_60 \
	-gencode=arch=compute_61,code=sm_61 \
	-gencode=arch=compute_70,code=sm_70 \
	-gencode=arch=compute_75,code=sm_75 \
	-gencode=arch=compute_75,code=compute_75 \

adj_matrix:adj_matrix_gpu.cu
	$(NVCC) $(ARCH) -std=c++11 -O3 -o adj_matrix adj_matrix_gpu.cu

clean:
	rm -f adj_matrix