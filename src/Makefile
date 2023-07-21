CXXFLAGS = -Wall -fPIC -I../steam/sdk/public
LDFLAGS = -shared

SOURCES = steam-call-manager.cpp ez-steam-api.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = ez-steam-api.so

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o *.so
