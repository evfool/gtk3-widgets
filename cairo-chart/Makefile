all: main

FILES = *.c

PKGS = `pkg-config --cflags --libs gtk+-3.0 glib-2.0` -lm

main: $(FILES)
	gcc -g -o $@ -Wall $(FILES) $(PKGS)

