#ifndef AUXILIARY_FUNCTION_H
#define AUXILIARY_FUNCTION_H

#include <string>
#include <iostream>

using namespace std;

string size_to_string(int n, int tam)
{
	string size = to_string(n);
	if(size.length() < tam)
	{
		size = string(tam - size.length(), '0') + size;
    }

    return size;
}

vector<string> split_into_words(string Mensaje)
{
    vector<string> res;
    int ianterior = 0;
    for(int i = 0; i < Mensaje.length(); i++){
        i = Mensaje.find(" ", i);
        if(i != -1)
        {
            res.push_back(Mensaje.substr(ianterior, i - ianterior));
            ianterior = i+1;
        }
        else
        {
            res.push_back(Mensaje.substr(ianterior));
            i = Mensaje.length();
        }
    }
    
    return res;
}

#endif