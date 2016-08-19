CC      = gcc
CX      = c++
LD      = ld
AR      = ar
AS      = as
NM      = nm
STRIP   = strip
RANLIB  = ranlib
DLLTOOL = dlltool
OBJDUMP = objdump
RESCOMP = windres

#CFLAGS = -Wl,--subsystem,windows -mwindows -DWINVER=0x0400 -D__WIN95__ -D__GNUWIN32__ -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__

CFLAGS += -DCONFIG_WIN32
#LFLAGS = -lregex -lpng -ljpeg -lzlib -ltiff -lstdc++ -lgcc -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lctl3d32 -lodbc32 -ladvapi32 -lodbc32 -lwsock32 -lopengl32 -lglu32 -lole32 -loleaut32 -luuid

#LFLAGS = -lwsock32
LFLAGS += -lws2_32 -lcomctl32 -liconv

TARGET = qqClick.exe

OBJS = qqClick.o utility.o
RCOBJS = qqClick.rc.o

.PHONY: $(RCOBJS)

all: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS) $(RCOBJS)
	# add -mwindows to prevent from cmd console
	#$(CC) -mwindows -static -o $@ $^ $(LFLAGS)
	$(CC) $(CFLAGS) -static -o $@ $^ $(LFLAGS)

$(RCOBJS):
	$(RESCOMP) -o $(RCOBJS) qqClick.rc

clean:
	rm -f $(RCOBJS) $(OBJS) *.exe
