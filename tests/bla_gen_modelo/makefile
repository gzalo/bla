all: bla_test_modelo.exe

bla_test_modelo.exe: bla_test_modelo.cpp
	g++ $< -o $@ -O2 -Wall
	
.PHONY: run clean

run:
	bla_test_modelo.exe

clean:
	del bla_test_modelo.exe
	
