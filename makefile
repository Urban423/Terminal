Target = silence
CC = g++
build = ./build/
OS = linux
FORMAT = cpp


bin = ./bin/
folders += ${bin}
folders += ${build}
folders += log

SCR += ${wildcard *.$(FORMAT)}
OBJ += ${patsubst %.$(FORMAT),${bin}%.o, ${SCR}}
OBJ_DEL += ${subst /,\, ${OBJ}}
FOLDERS  = ${subst /,\, ${folders}}

DFlags += -D $(OS)
CFlags += -O3 
LFlags += -L ./ -lz


ifeq ($(OS),windows)
    MKDIR = @for %%d in ($(FOLDERS)) do if not exist %%d mkdir %%d
	DEL= del
else
    MKDIR = for dir in $(folders) ; do if [ ! -d $$dir ]; then mkdir -p $$dir; fi; done
	DEL = rm -f
endif



buildApp: binFile clear appBuilder

appBuilder: ${OBJ}
	${CC} -o ${build}${Target} $^ ${resources} ${Libs} ${CFlags} ${LFlags}

libBuilder: ${OBJ}
	ar rcs ${libpath}lib${Target}.a ${OBJ}

binFile:
	$(MKDIR)

${bin}%.o: %.$(FORMAT)
	${CC} ${IFlags} ${DFlags} -o $@ -c $< ${CFlags}
	
clear:
	${DEL} ${OBJ}
