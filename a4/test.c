#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int badinput_check(char* input){
	int valid = 0, i =0, ascii;

	for(i = 0; i < strlen(input); i ++){
		ascii = input[i];
		printf("ascii[%d]: %c\n", i, ascii);
		if (ascii >= 65 && ascii <= 90) valid = 1;
		else if (ascii == 32) valid = 1;
		else if (ascii >= 97 && ascii <= 122) valid = 1;
		else valid = 0;
	}
	return valid;
}

int main(int argc, char* argv[]){
	char buffer[8];

	char* mykey = malloc(sizeof(char)*32);
	memset(mykey, '\0', 32);
	FILE *key_ptr = fopen("mykey", "r");
	fgets(mykey, 32, key_ptr);
	fclose(key_ptr);
	mykey[strcspn(mykey, "\n")] = '\0';

	int i = 0;
 	for(i = 0; i < 2; i ++){
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		memcpy(buffer, (mykey + 7*i), 7);
		printf("string: \n%s\n", buffer);
	}
	printf("%s\n", mykey);

	free(mykey);
	return 0;
}
