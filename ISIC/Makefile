CC=g++
BIN=Normalize_Image Compute_AvgDistance Recover_Image Compress_Mat Recover_Mat Compute_LSH Compute_ValueErr Compute_ValueErr_V2
#TestAll
FLAGS=-Wall -g -fpermissive -std=c++0x -O3
RM=-rm -f
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
COMMON_OBJ=$(filter-out Normalize_Image.o Recover_Mat.o Compute_LSH.o Compute_AvgDistance.o Compute_ValueErr.o Compute_ValueErr_V2.o Compress_Mat.o Recover_Image.o,$(OBJ))

#thrift
#THRIFT_LINK=-lthrift
#THRIFT_LIB=-L/usr/local/lib

#openssl
OPENSSL_LINK=-lcrypto

#KL1p
KL1P_LINK=-lKLab64
KL1P_LIB=-L/data/software/KL1p-0.4.2/bin/unix
KL1P_INC=-I/data/software/KL1p-0.4.2/include -I

#OPENCV
OPENCV_LIB=`pkg-config opencv --libs`
OPENCV_INC=`pkg-config opencv --cflags`

#caravel
CARAVEL_LINK=-lcaravel
CARAVEL_LIB=-L../Caravel
CARAVEL_INC=-I../Caravel

#pthread
PTHREAD_LINK=-pthread

#LIBDL
DL_LINK=-ldl

#combine the inc & lib
INC=$(OPENSSL_INC) $(CARAVEL_INC) $(KL1P_INC) $(OPENCV_LIB)
LIB=$(CARAVEL_LINK) $(CARAVEL_LIB) $(OPENSSL_LINK) $(KL1P_LIB) $(OPENCV_LIB) $(KL1P_LINK) $(PTHREAD_LINK) $(DL_LINK)

all: $(BIN)

%.d: %.cpp
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

Normalize_Image:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Recover_Image:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Recover_Mat:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Compress_Mat:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Compute_AvgDistance:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Compute_LSH:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Compute_ValueErr:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Compute_ValueErr_V2:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(BIN)

-include $(SRC:.cpp=.d)
