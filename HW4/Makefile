all: main clean

main:
	gcc -c StaticLib.c
	ar -rc libStatic.a StaticLib.o
	ranlib libStatic.a
	gcc -fPIC -c FirstDynamicLib.c
	gcc -shared -o libFirstDynamic.so FirstDynamicLib.o
	gcc -fPIC -c SecondDynamicLib.c
	gcc -shared -o libSecondDynamic.so SecondDynamicLib.o
	gcc -c Main.c
	gcc Main.o -ldl -L. -lFirstDynamic -lStatic -o result -Wl,-rpath=./

clean:
	rm -rf *.o
