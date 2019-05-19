//  Assignment: OTP
//  File Name: keygen.c
//  Author: Zongzhe Bi
//  Class: CS 344
//  Created by Zongzhe Bi on 11/25/18.
//  Copyright © 2018 Zongzhe Bi. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]){
    srand((unsigned)time(NULL));
    int i = 0;
    int charactor = 0;

    if( argc != 2 ){
        perror("Invalid Input\n");
        exit(1);
    }

    for(i = 0; i < atoi(argv[1]); i++){
        charactor = rand()%27 + 65;
        if (charactor == 91) charactor = 32;
        printf("%c", charactor);
    }

    printf("\n");
    return 0;
}
