pwd := env_var('PWD')

clean:
	rm -rf ./build
	rm -rf ./release
	rm -rf ./dist

[unix]
setup:
	meson setup build

[windows]
setup:
	meson setup build --backend vs

build:
	meson compile -C build

[unix]
release: clean
	meson setup release -Dbuildtype=release
	meson compile -C release

[windows]
release: clean
	meson setup release -Dbuildtype=release --backend vs
	meson compile -C release

[unix]
dist: clean release
	mkdir -p ./dist
	cp -r ./release/cubegame ./dist
	cp -r ./assets ./dist

[windows]
dist: clean release
	mkdir -p ./dist
	cp -r ./release/cubegame.exe ./dist
	cp -r ./assets ./dist

[unix]
run: build
	./build/cubegame

[windows]
run: build
	./build/cubegame.exe

