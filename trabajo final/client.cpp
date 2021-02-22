#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <map>
#include <iterator>
#include <queue>

#include "auxiliary_function.h"
#include "client.h"

using namespace std;

Client main_client;

void show_menu()
{
    cout<<"DISTRIBUTED DATABASE"<<endl;
    string input_text;

    while(true)
    {
        cout<<"SQL: ";
        getline(cin, input_text);
        
        vector<string> words_input_text = split_into_words(input_text);

        main_client.parse_input(words_input_text);
    }
}

int main()
{    
    main_client.connect_with_slaves();
    show_menu();
    return 0;
}