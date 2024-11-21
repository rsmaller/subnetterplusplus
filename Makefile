subnet: # First label runs first as default.
	@g++ -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -o subnet ./subnetter.c++ 

clean: # Not first label, will not run unless called via make.
	@rm subnet.exe