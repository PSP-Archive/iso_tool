TARGET = iso_tool

PSP_FW_VERSION = 500

#USE_KERNEL_LIBC = 1

#USE_KERNEL_LIBS = 1

#CLASSG_LIBS = libs

VERSION = 1.53

INCDIR = $(CLASSG_LIBS)
CFLAGS = -O3 -G0 -Wall -fno-builtin-printf -mno-check-zero-division -DVERSION=$(VERSION)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ./
LDFLAGS = 
LIBS = -lpspsystemctrl_user -lpspsystemctrl_kernel -lpsppower -lpspkubridge -lz -lpspumd -lpspgu -lpsputility -lpsprtc -lpspaudiolib -lpspaudio -lpspnet -lpsphttp
LIBS += -lcurl -lpspgum -lm


OBJS = 	main.o \
		msg.o \
		file.o \
		ciso.o \
		key.o \
		fnt_print.o \
		screen.o \
		shnm16psp.o \
		sound.o \
		menu.o \
		iso.o \
		unicode.o \
		umd.o \
		web.o \
		error.o \
		pspDecrypt.o

#		web.o \

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = iso_tool $(VERSION)
PSP_EBOOT_ICON = iso_tool.png


PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm -f ./PARAM.SFO
	rm -f ./main.o
	rm -f ./iso_tool.elf

main.o: main.c main.h file.h error.h pspdecrypt.h screen.h sound.h menu.h

file.o: file.c main.h file.h error.h ciso.h umd.h macro.h

ciso.o: ciso.c ciso.h file.h error.h

key.o: key.c key.h

fnt_print.o: fnt_print.c fnt_print.h

screen.o: screen.c screen.h menu.h file.h error.h main.h key.h fnt_print.h shnm16psp.h sound.h unicode.h web.h msg.h

shnm16psp.o:shnm16psp.c shnm16psp.h

sound.o: sound.c sound.h beep_data.c

menu.o: menu.c menu.h file.h error.h main.h screen.h pspdecrypt.h sound.h iso.h unicode.h fnt_print.h ciso.h umd.h key.h web.h msg.h msg.c macro.h

iso.o: iso.c iso.h file.h

unicode.o: unicode.c unicode.h

umd.o: umd.c umd.h

error.o: error.c error.h screen.h

web.o: web.c web.h

msg.o: msg.c msg.h

dummy:

	