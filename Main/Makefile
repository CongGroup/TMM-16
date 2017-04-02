CC=g++
BIN=TestMain BuildIndex QueryThroughput BuildThroughput
#TestAll
FLAGS=-Wall -g -fpermissive -std=c++0x -O3
RM=-rm -f
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
COMMON_OBJ=$(filter-out TestMain.o QueryThroughput.o BuildThroughput.o BuildIndex.o,$(OBJ))

#openssl
OPENSSL_LINK=-lcrypto

#caravel
CARAVEL_LINK=-lcaravel
CARAVEL_LIB=-L../Caravel
CARAVEL_INC=-I../Caravel

#combine the inc & lib
INC=$(OPENSSL_INC) $(THRIFT_INC) $(CARAVEL_INC) $(KL1P_INC)
LIB=$(CARAVEL_LINK) $(CARAVEL_LIB) $(OPENSSL_LINK)


all: $(BIN)

%.d: %.cpp
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

TestMain:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

BuildIndex:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

BuildThroughput:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

QueryThroughput:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(BIN)

-include $(SRC:.cpp=.d)
