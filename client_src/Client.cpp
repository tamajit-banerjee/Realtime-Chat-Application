#include "../utils/Header.hpp"

using namespace std;

string username;

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}


void *send_msg_handler( void* client_send_socket ) {

  int sockfd = *(int*)client_send_socket;
  char buffer[BUFFER_SZ];

  while( 1 ) {

    char wrote[200] = {};
    char buffer[BUFFER_SZ];
    string to_whom = "", message = "";

    /*
      taking input 
    */
  
    str_overwrite_stdout();
    fgets(wrote, 200, stdin);
    string line(wrote);

    if( line == "" || line == " " || line == "\n")
        continue;

    regex msg_format("@.+\\s.+\n*");

    if (!regex_match(line, msg_format)){
        cout<<"Invalid line. Please write the message in format: @[recepient] [msg]\n";
        continue;
    }

    bool found_space;
    int cur = 0;
    while( line[cur]!= ' ')
      ++cur;
    

    for (int i=1;i<line.length();i++){
        if ( i < cur ){
          to_whom += line[i];
        }else if( i > cur ){
          message += line[i];
        }
    }

    /*
      sending the message
    */

    string s = "SEND " + to_whom + "\nContent-length: " + to_string(message.length()) + "\n\n" + message; 
    cout<<"MESSAGE BEING SENT :: \n"<<s<<"\n";
    char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
    send(sockfd, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
    
    /*
      receiving response from the server
    */

    bzero(buffer,BUFFER_SZ);
    recv( sockfd , buffer , BUFFER_SZ , 0 );
    string rv(buffer);
    cout<<"MESSAGE RECEIVED FROM SERVER UPON SENDING :: \n"<<rv<<endl;

    if( rv == "ERROR 103 Header Incomplete\n\n"){
        close(sockfd);
        exit(0);
    }

  }

}

void *recv_msg_handler(  void* client_receive_socket ){

    //regex RECEIVE_FORMAT_OK("FORWARD\\s[0-9a-zA-Z]+\nContent-length:\\s\\d+\n\n.*");

    int sockfd = *(int*)client_receive_socket;

	char buffer[BUFFER_SZ] ;

    while (1) {
    
      bzero(buffer, BUFFER_SZ);

    /*
      receiving message from the server
    */
        
      int receive = recv(sockfd, buffer , BUFFER_SZ , 0);

      string rv(buffer);

    if( rv.size() == 0 ){
      exit(0);
    }

    cout<<"MESSAGE RECEIVED FROM SERVER WHICH WAS SENT BY ANOTHER USER ::\n "<<rv<<endl;

    /*
      parsing the message
    */

    // if( regex_match( rv , RECEIVE_FORMAT_OK)){

    //     string s = "ERROR 103 Header Incomplete\n\n"; 
    //     char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
    //     send(sockfd , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
    //     continue;

    // }

    bool f = false;
    string sender="";
    
    int cur = 8;
    if( rv.substr(0,8) != "FORWARD ")
        f = true;
    while( rv[cur]!='\n' ){
        sender += rv[cur];
        cur++;
    }
    cur++;

    string c = "Content-length: ";
    if( rv.substr(cur,c.size()) != c)
        f = true;
    string message_len = "";
    while(rv[cur]!='\n'){
        cur++;
        if( rv[cur]-'0' >= 0 && rv[cur]-'0' <= 9 )
            message_len+=rv[cur];
    }

    int m_len = stoi(message_len);

    cur+=2;

    string message = rv.substr(cur,rv.length()-cur);

    if( m_len != message.length() || f ){

      string s = "ERROR 103 Header Incomplete\n\n"; 
      char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
      send(sockfd, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
      continue;

    }

    /*
      output the message
    */

    cout<<"New Message from "<< sender<<" | "<<message<<"\n";

    string s = "RECEIVED "+sender+"\n\n";

    char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 

    send(sockfd, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);

    }

}


int main(){

    int port = PORT;
    int client_receive_socket , client_send_socket;

    cout<<"WELCOME TO CHAT ROOM\n\n";

    struct sockaddr_in server_address ;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    while(1){

        cout<<"Please Enter your username in format [0-9a-zA-Z]+  :: \n";
        cin>>username;


        /*
          Registering to send 
        */

        client_send_socket = socket(AF_INET,SOCK_STREAM,0);

        int send_socket_connection_status = connect( client_send_socket , (struct sockaddr *)&server_address, sizeof(server_address));
        
        if ( send_socket_connection_status  == -1) {
                printf("ERROR: SENDING SOCKET CREATING FAILED \n");
                return EXIT_FAILURE;
        }

        string header = "REGISTER TOSEND ";
        string s = header + username + "\n\n";

        char buffer[BUFFER_SZ];
        char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str());
        

        send( client_send_socket , SEND_MESSAGE , 64, 0 );
        bzero(buffer, BUFFER_SZ);
        recv( client_send_socket , buffer , BUFFER_SZ, 0 );

        string rv1(buffer);

        cout<<"MESSAGE RECIEVED AFTER REGISTER TOSEND :: \n";
        cout<<rv1;

        if( rv1 != "REGISTERED TOSEND "+username+"\n\n")
            continue;


        /*
          Registering to receive 
        */

        client_receive_socket = socket(AF_INET,SOCK_STREAM,0);

        int receive_socket_connection_status = connect( client_receive_socket , (struct sockaddr *)&server_address, sizeof(server_address));
        
        if ( receive_socket_connection_status  == -1) {
                
                printf("ERROR: RECEIVING SOCKET CREATING FAILED \n");
                return EXIT_FAILURE;

        }

        header = "REGISTER TORECV ";

        s = header + username + "\n\n";

        SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str());
      
        send( client_receive_socket , SEND_MESSAGE , 64, 0 );
        bzero(buffer, BUFFER_SZ);
        recv( client_receive_socket , buffer , BUFFER_SZ, 0 );

        string rv(buffer);

        cout<<"MESSAGE RECIEVED AFTER REGISTER TORECV :: \n";
        cout<<rv;

        if( rv != "REGISTERED TORECV "+username+"\n\n")
            continue;

        break;
    }

    /*
    creating the threads fro sending and receiving 
    */
    
  pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL,  &send_msg_handler, &client_send_socket ) != 0){
          printf("ERROR: pthread\n");
          return EXIT_FAILURE;
  }


	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL,  &recv_msg_handler, &client_receive_socket ) != 0){
          printf("ERROR: pthread\n");
          return EXIT_FAILURE;
  }

  while(1){

  }

  return 0;

}