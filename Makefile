##
## compiling under ubuntu:
##  for compiling linux: make 
##  for compiling win32: make OSTYPE=win32
##

ifeq ($(OSTYPE),win32)
	CPP = i686-w64-mingw32-gcc -m32
	AR = i686-w64-mingw32-ar rc
	RANLIB = i686-w64-mingw32-ranlib
	LINKFLAGS = -mdll -lm -lwsock32 -lws2_32 -Xlinker --add-stdcall-alias -s
	DLLEND = .dll
	ZLIB_OSFLAGS =
else
	CPP = gcc -m32
	AR = ar rc
	RANLIB = ranlib
	ARCHFLAG = -fPIC
	LINKFLAGS = -fPIC -shared -ldl -lm -s
	DLLEND = .so
	ZLIB_OSFLAGS = -DNO_UNDERLINE -DZ_PREFIX
endif

TARGET = jk_botti_mm
BASEFLAGS = -Wall -Wno-write-strings
ARCHFLAG += -march=i686 -mtune=generic -msse -msse2 -mmmx -pipe -mfpmath=sse

ifeq ($(DBG_FLGS),1)
	OPTFLAGS = -O0 -g
else
	OPTFLAGS = -O2 -fomit-frame-pointer -g
	OPTFLAGS += -funsafe-math-optimizations
	LTOFLAGS = -flto -fvisibility=hidden
	LINKFLAGS += ${OPTFLAGS} ${LTOFLAGS}
endif

INCLUDES = -I"./metamod" \
	-I"./common" \
	-I"./dlls" \
	-I"./engine" \
	-I"./pm_shared"

CFLAGS = ${BASEFLAGS} ${OPTFLAGS} ${ARCHFLAG} ${INCLUDES}
CPPFLAGS = -fno-rtti -fno-exceptions ${CFLAGS} 

SRC = 	bot.cpp \
	bot_chat.cpp \
	bot_client.cpp \
	bot_combat.cpp \
	bot_config_init.cpp \
	bot_models.cpp \
	bot_navigate.cpp \
	bot_query_hook.cpp \
	bot_query_hook_linux.cpp \
	bot_query_hook_win32.cpp \
	bot_skill.cpp \
	bot_sound.cpp \
	bot_weapons.cpp \
	commands.cpp \
	dll.cpp \
	engine.cpp \
	h_export.cpp \
	safe_snprintf.cpp \
	util.cpp \
	waypoint.cpp

OBJ = $(SRC:%.cpp=%.o)

${TARGET}${DLLEND}: zlib/libz.a ${OBJ} 
	${CPP} -o $@ ${OBJ} zlib/libz.a ${LINKFLAGS}
	cp $@ addons/jk_botti/dlls/

zlib/libz.a:
	(cd zlib; AR="${AR}" RANLIB="${RANLIB}" CC="${CPP} ${OPTFLAGS} ${ARCHFLAG} ${ZLIB_OSFLAGS} -DASMV" ./configure; $(MAKE) OBJA=match.o; cd ..)

clean:
	rm -f *.o ${TARGET}${DLLEND} Rules.depend zlib/*.exe 
	(cd zlib; $(MAKE) clean; cd ..)
	rm -f zlib/Makefile

distclean:
	rm -f Rules.depend ${TARGET}.dll ${TARGET}.so addons/jk_botti/dlls/* zlib/*.exe
	(cd zlib; $(MAKE) distclean; cd ..)

#waypoint.o: waypoint.cpp
#	${CPP} ${CPPFLAGS} -funroll-loops -c $< -o $@

#safe_snprintf.o: safe_snprintf.cpp
#	${CPP} ${CPPFLAGS} -funroll-loops -c $< -o $@

%.o: %.cpp
	${CPP} ${CPPFLAGS} ${LTOFLAGS} -c $< -o $@

%.o: %.c
	${CPP} ${CFLAGS} ${LTOFLAGS} -c $< -o $@

depend: Rules.depend

Rules.depend: Makefile $(SRC)
	$(CPP) -MM ${INCLUDES} $(SRC) > $@

include Rules.depend
