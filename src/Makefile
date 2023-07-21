# Release flags: -flto -Os
# Debug flags: -g -O0
CXXFLAGS = $(CXXFLAGS) -Wall -fPIC -I../steam/sdk/public

# Release flags: -s
LDFLAGS = $(LDFLAGS) -shared

SOURCES = steam-call-manager.cpp ez-steam-api.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = ez-steam-api.so

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o *.so
