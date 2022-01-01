SHELL=cmd
CXX=g++
CDEFFLAGS=-std=c++20 -Wall -Wextra -Wpedantic -Wconversion
CDEBFLAGS=-g -O0 -D _DEBUG
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -mwindows -D NDEBUG -static -fno-rtti
LIB=-municode -ld2d1 -luuid -ldwrite -lgdi32 -lwinmm

SRC=src
OBJ=obj
DOBJ=obj.d

TARGET=SnakeD2D

default: debug

$(OBJ):
	mkdir $(OBJ)
$(DOBJ):
	mkdir $(DOBJ)

$(OBJ)/resource.rc.o: $(SRC)/resource.rc
	windres -i $^ $@ -D NDEBUG
$(OBJ)/%.cpp.o: $(SRC)/%.cpp
	$(CXX) -c $^ -o $@ $(CDEFFLAGS) $(CFLAGS)

$(DOBJ)/resource.rc.o: $(SRC)/resource.rc
	windres -i $^ $@ -D _DEBUG
$(DOBJ)/%.cpp.o: $(SRC)/%.cpp
	$(CXX) -c $^ -o $@ $(CDEFFLAGS) $(CDEBFLAGS)


srcs = $(wildcard $(SRC)/*.cpp)
srcs += $(wildcard $(SRC)/*.rc)
srcs := $(subst $(SRC)/,,$(srcs))
objs_d = $(srcs:%=$(DOBJ)/%.o)
objs_r = $(srcs:%=$(OBJ)/%.o)

debug_obj: $(objs_d)
	$(CXX) $^ -o deb$(TARGET).exe $(CDEBFLAGS) $(LIB)

debug: $(DOBJ) debug_obj

release_obj: $(objs_r)
	$(CXX) $^ -o $(TARGET).exe $(CFLAGS) $(LIB)

release: $(OBJ) release_obj


debbulk: $(DOBJ) bulkcompile.cpp $(DOBJ)/resource.rc.o
	$(CXX) bulkcompile.cpp $(DOBJ)/resource.rc.o -o deb$(TARGET).exe $(CDEFFLAGS) $(CDEBFLAGS) $(LIB)

relbulk: $(OBJ) bulkcompile.cpp $(OBJ)/resource.rc.o
	$(CXX) bulkcompile.cpp $(OBJ)/resource.rc.o -o $(TARGET).exe $(CDEFFLAGS) $(CFLAGS) $(LIB)

clean:
	del *.exe
	IF EXIST $(OBJ) rd /s /q $(OBJ)
	IF EXIST $(DOBJ) rd /s /q $(DOBJ)

