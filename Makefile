
PROGS := xpanic
LDLIBS := -lX11

all: $(PROGS)

build-dep:
	sudo apt-get -y install gcc libx11-dev

clean:
	rm -f $(PROGS)
