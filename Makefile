.SILENT:
subnet: # First label runs first as default.
	g++ -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -std=c++11 -o subnet ./subnetter.cpp 

clean: # Not first label, will not run unless called via make.
	-rm -f subnet subnet.exe
