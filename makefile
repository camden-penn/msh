PROJECT=msh
DEPENDENCIES=process
HEADERS= $(PROJECT:=.h)  $(DEPENDENCIES:=.h)
OBJ_DIRECTORY=obj
SRC_EXT=.cpp #Either .cpp or .c, depending on project language
COMPILER=g++
COMPILE_FLAGS=-Wall -std=c++11
$(PROJECT): $(OBJ_DIRECTORY) $(OBJ_DIRECTORY)/$(PROJECT:=.o) $(OBJ_DIRECTORY)/$(DEPENDENCIES:=.o)
	$(COMPILER) $(COMPILE_FLAGS) $</$(DEPENDENCIES:=.o) $</$(PROJECT:=.o) -o $@
$(OBJ_DIRECTORY):
	mkdir $@
$(OBJ_DIRECTORY)/%.o: %$(SRC_EXT) $(HEADERS)
	$(COMPILER) $(COMPILE_FLAGS) -c $< -o $@ 
.PHONY: clean
clean:
	-rm -r $(OBJ_DIRECTORY)
	-rm $(PROJECT)
