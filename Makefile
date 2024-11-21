subnet: # First label runs first as default.
	@g++ -o subnet ./subnetter.c++ 

clean: # Not first label, will not run unless called via make.
	@rm subnet.exe