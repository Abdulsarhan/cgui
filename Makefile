.PHONY: all clean

EXE = cgui

all: $(EXE)

%:%.c
	cc -ggdb -o $@ $< -lX11 -lX11-xcb -lxcb  -lGL

clean:
	rm -f $(EXE)
