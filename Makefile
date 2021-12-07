compile: 
	@g++-11 Server.cpp -o server 
	@g++-11 Client.cpp -o client
	@g++-11 Error_Client.cpp -o error_client
	$(info Compiling code...)

addclient:
	@./client
	$(info Creating new client...)

adderror_client:
	@./error_client
	$(info Creating new error_client...)

startserver:
	@./server
	$(info Starting server...)