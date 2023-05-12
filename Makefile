CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/usr/include/cppconn
LDFLAGS = -L/usr/lib -lmysqlcppconn -lpthread

TARGET = testapp
SRC = test.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

