CXX ?= c++
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -Werror

TARGET = ttfx-text-test
SOURCES = TTFXTextTest.cpp ../src/TTFXTextArt.cpp ../src/TTFXEngine.cpp

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
