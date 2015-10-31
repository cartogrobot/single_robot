/*
 * web_sim_test.cpp
 * Author: Aven Bross
 * Date: 10/4/2015
 * 
 * Testing web simulation functionality
 */
 
#include "web_sim_server.h"
#include <iostream>
#include <cstring>
#include <cstdio>

using std::cout;
using std::cin;

int main(){
	WebSimServer s(9999);
    s.start();
    
    while(1){
        if(s.isDead()){
            s.start();
        }
        std::cin.ignore();
    }
    return 0;
}
