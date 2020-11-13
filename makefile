TARGET=ts
TARGET2=tc
LDLIBS+=-pthread

all: $(TARGET) $(TARGET2)

clean:
	rm -f $(TARGET) *.o
	rm -f $(TARGET2) *.o
