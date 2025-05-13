CXX = g++
NVCC=nvcc
ARCH=-gencode=arch=compute_52,code=sm_52 \
	-gencode=arch=compute_60,code=sm_60 \
	-gencode=arch=compute_61,code=sm_61 \
	-gencode=arch=compute_70,code=sm_70 \
	-gencode=arch=compute_75,code=sm_75 \
	-gencode=arch=compute_75,code=compute_75 \

LDFLAGS_PERSISTENT = -lpmemobj++ -lpmemobj -lpmem -pthread

PEMEM_PATH = /mnt/pmem2-ext4-dax/par.pmem
LETTER_PATH = ../../datasets/letter/letter.scale

all:main.cpp
	$(CXX)  $^ -o $@ $(LDFLAGS_PERSISTENT)

rp:
	sudo rm $(PEMEM_PATH)

make_run:main.cpp
	$(CXX)  $^ -o $@ $(LDFLAGS_PERSISTENT)
	sudo ./all $(PEMEM_PATH) $(LETTER_PATH)

run:
	sudo ./all $(PEMEM_PATH) $(LETTER_PATH)

clean:
	rm all & rm make_run	