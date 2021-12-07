compile: 
	mkdir executables
	@g++-11 server_src/Server.cpp -o executables/server 
	@g++-11 client_src/Client.cpp -o executables/client
	@g++-11 client_src/Error_Client.cpp -o executables/error_client
	$(info Compiling code...)

addclient:
	@./executables/client
	$(info Creating new client...)

adderror_client:
	@./executables/error_client
	$(info Creating new error_client...)

startserver:
	@./executables/server
	$(info Starting server...)