all: build

run:
	@build/brewmenu

build: clean
	@mkdir -p build
	@gcc src/main.c -o build/brewmenu

clean:
	@rm -rf build/