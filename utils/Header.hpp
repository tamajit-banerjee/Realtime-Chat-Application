
#include <iostream>
#include <ostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <map>
#include <regex>

using namespace std;

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define PORT 9974