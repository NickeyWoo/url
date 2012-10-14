
TARGET := url

all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

$(TARGET): main.o logs.o misc.o hashtable.o
	g++ $^ -o $@ -L../../project/commlib64/baselib/ -L../../project/cityhash-1.0.3/src/ -L/usr/local/x84_64-linux-gnu/ -lcityhash -lcomm_oi -lcrypto -lboost_regex

%.o: %.cpp
	g++ $^ -c -o $@ -I../../project/cityhash-1.0.3/src -I../../project/commlib64/baselib

