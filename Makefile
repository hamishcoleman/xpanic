
PROGS := xpanic
CFLAGS := -lX11

all: $(PROGS)

build-deps:
	sudo apt-get -y install libx11-dev

clean:
	rm -f $(PROGS)
