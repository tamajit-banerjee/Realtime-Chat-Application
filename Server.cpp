#include "Header.hpp"

map<string ,pair< int,int > > hashmap; 

/* Send message to the appropriate client using Stop-and-Wait method */
int send_message(string message, string from_user, string to_user ){
    
    char buffer[BUFFER_SZ];
    string s = "FORWARD "+ from_user + "\nContent-length: "+ to_string(message.length())+ "\n\n"+message; 

    cout<<"MESSAGE SENT TO CLIENT FROM SERVER ::\n"<<s<<"\n";

    // sending to all 
    if( to_user == "ALL" ){
        map<string,pair<int,int> > ::iterator it;
        bool unable = false;
        for( it = hashmap.begin() ; it !=hashmap.end() ;it++){
            if( it->first == from_user )
                continue;
            cout<<"CURRENTLY SENDING MESSAGE TO :: "<<it->first<<"\n";
            pair<int,int> temp =  it->second;
            char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
            int send_num = send(temp.second, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
            if( send_num < 0){
                unable = true;
            }
            bzero(buffer, BUFFER_SZ);
            recv(temp.second, buffer , BUFFER_SZ , 0 );
            string m(buffer);
            if( m != "RECEIVED "+from_user+"\n\n"){
                unable = true;
            }
            cout<<"MESSAGE SENT FROM CLIENT AFTER SENDING MESSAGE :: "<<m<<"\n";
            }
        if( unable )
            return 0;
    }else{
        // sending to one
        if( hashmap.find(to_user) == hashmap.end() ){
            return 0;
        }
        pair<int,int> temp =  hashmap[to_user];
        char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
        cout<<"CURRENTLY SENDING MESSAGE TO :: "<<to_user<<"\n";
        int send_num = send(temp.second, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
        if( send_num < 0){
            return 0;
        }
        bzero(buffer, BUFFER_SZ);
        recv(temp.second, buffer , BUFFER_SZ , 0 );
        string m(buffer);
        cout<<"MESSAGE SENT FROM CLIENT AFTER SENDING MESSAGE :: "<<m<<"\n";
        if( m != "RECEIVED "+from_user+"\n\n"){
            return 0;
        }
    }
    return 1;
}



void *handle_client(void* server_socket){

    string to_send = "REGISTER TOSEND ";
    string to_recieve = "REGISTER TORECV ";
    regex alphanum("[0-9a-zA-Z]+");
    //regex SEND_FORMAT_OK("SEND\\s[0-9a-zA-Z]+\nContent-length:\\s\\d+\n\n.*");
    char buff_out[BUFFER_SZ];
    int connect_status = *(int*)server_socket;
    int receive = recv(connect_status, buff_out, BUFFER_SZ, 0);
    string rec(buff_out);
    string username;

    if( strstr(rec.c_str(),to_send.c_str()) ){

        int end = rec.length();

        username = rec.substr(16,end-18);

        cout<<"REGISTER TOSEND RECEIVED :: "<<username<<"\n";

        if (!regex_match(username, alphanum)){
            string s = "ERROR 100 Malformed username\n\n";
            cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
            char* ERROR_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str());
            send(connect_status , ERROR_MESSAGE , strlen(ERROR_MESSAGE), 0);
            close(connect_status);
            pthread_detach(pthread_self());
            return NULL;
        }

        if( hashmap.find(username) == hashmap.end() ){
            pair<int,int> p = {connect_status , -1};
            hashmap[username] = p;
        }else{
            pair<int,int> p = hashmap[username];
            p.first = connect_status;
            hashmap[username] = p;
        }

        string s = "REGISTERED TOSEND "+username+"\n\n" ; 

        char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 

        send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);

    }else if( strstr(rec.c_str(),to_recieve.c_str()) ){

        int end = rec.length();
        username = rec.substr(16,end-18);

        cout<<"REGISTER TORECV RECEIVED :: "<<username<<"\n";

        if (!regex_match(username, alphanum)){
            string s = "ERROR 100 Malformed username\n\n";
            cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
            char* ERROR_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str());
            send(connect_status , ERROR_MESSAGE , strlen(ERROR_MESSAGE), 0);
            close(connect_status);
            pthread_detach(pthread_self());
            return NULL;
        }

        if( hashmap.find(username) == hashmap.end() ){
            pair<int,int> p = {-1,connect_status};
            hashmap[username] = p;
        }else{
            pair<int,int> p = hashmap[username];
            p.second = connect_status;
            hashmap[username] = p;
        }

        string s = "REGISTERED TORECV " + username + "\n\n" ; 
        cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
        char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
        send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);

        pthread_detach(pthread_self());
        return NULL;

    }else{

        string s = "ERROR 101 No user registered\n\n"; 
        cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
        char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
        send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
        close(connect_status);

        pthread_detach(pthread_self());
        return NULL;

    }


	while(1){

        bzero(buff_out, BUFFER_SZ);

		int receive = recv(connect_status, buff_out, BUFFER_SZ, 0);

        string rv(buff_out);

        cout<<"MESSAGE RECEIVED FROM USER "<<username<<" :: \n"<<rv<<"\n";

        if( hashmap[username].second == -1 ){
            string s = "ERROR 101 No user registered\n\n"; 
            char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
            send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
            continue;
        }

        bool f = false;
        string to_send="";
        if( rv.substr(0,5) != "SEND ")
            f = true;
        int cur = 5;
        while( rv[cur]!='\n' ){
            to_send += rv[cur];
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
            cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
            char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
            send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
            close(connect_status);
            close(hashmap[username].second);
            hashmap.erase(username);
            pthread_detach(pthread_self());
            return NULL;

        }

        cout<<"THE MESSAGE WILL BE SENT TO :: " << to_send<<"\n";

        if(send_message(message, username ,to_send)){

            string s = "SENT "+to_send+"\n\n"; 
            cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
            char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
            send(connect_status, SEND_MESSAGE , strlen(SEND_MESSAGE), 0);


        }else{

            string s = "ERROR 102 Unable to send\n\n"; 
            cout<<"THE MESSAGE THE SERVER IS SENDING BACK TO THE SENDER ::\n"<<s<<"\n";
            char* SEND_MESSAGE = strcpy(new char[ s.length() + 1], s.c_str()); 
            send(connect_status , SEND_MESSAGE , strlen(SEND_MESSAGE), 0);
            continue;

        }

	}

    close(connect_status);
    close(hashmap[username].second);
    pthread_detach(pthread_self());

	return NULL;

}




int main(){

    //create a socket 
    int main_server_socket = socket(AF_INET,SOCK_STREAM,0);

    int port = PORT;

    //specify an address for the socket 
    struct sockaddr_in server_address , client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);


    int connect_status = bind( main_server_socket , (struct sockaddr *)&server_address, sizeof(server_address));

    if( connect_status == -1 ){
        cout<<"There was an error making connection to the remote socket\n \n";
        return EXIT_FAILURE;
    }

    if( listen(main_server_socket , 10 ) < 0 ){
        cout<<"ERROR, Listener\n \n";
        return EXIT_FAILURE;
    }

    cout<<"WELCOME TO CHAT ROOM SERVER\n \n";

    string to_send = "REGISTER TOSEND ";

    string to_recieve = "REGISTER TORECV ";

    regex alphanum("[0-9a-zA-Z]+");

    pthread_t tid;

    while( 1 ){

        char buff_out[BUFFER_SZ];

        socklen_t clilen = sizeof(client_address);

        int *socket_status = (int *)malloc(sizeof(int));

		*socket_status = accept(  main_server_socket , (struct sockaddr*)&client_address, &clilen);

        pthread_create(&tid, NULL, &handle_client, (void*)socket_status);

    }
    return 0;

}