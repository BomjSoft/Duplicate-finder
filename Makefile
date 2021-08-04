CXX = g++
CXXFLAGS = -fomit-frame-pointer -fexpensive-optimizations -flto -O3 -DUNICODE
LDLIBS = libBCL.a -lgdi32 -luser32 -lkernel32 -lcomctl32 -lcomdlg32 -lglu32 libpng.a -lz libjpeg.a

SOURCES = main.cpp MainForm.cpp PerceptiveHash.cpp Texture.cpp
HEADERS = MainForm.h PerceptiveHash.h Texture.h
OBJECTS = main.o MainForm.o PerceptiveHash.o Texture.o
TARGET = Duplicate_finder.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDLIBS) -mwindows

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@