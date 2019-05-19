/********************************************************
Name: biz.buildrooms.c
Author: Zongzhe Bi
Date: 10/18/2018
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

struct rooms {
   char room_name[9];
   char room_type[11];
   int num_connection;
   char connections[6][9];
};

/*
Protocol parts
*/
struct rooms* buildroom();
void free_rooms(struct rooms*);
int IsSameRoom(struct rooms*, struct rooms*);
void ConnectRoom(struct rooms*, struct rooms*);
int ConnectionAlreadyExists(struct rooms*, struct rooms*);
int CanAddConnectionFrom(struct rooms*);
struct rooms* GetRandomRoom(struct rooms**);
int IsGraphFull(struct rooms**);
void AddRandomConnection(struct rooms**);
void write_file(struct rooms**);

/*
Functi Name: Main function.
Description: Control the whole proccess for create room files.
*/
int main(int argc, char const *argv[]) {
  srand((unsigned)time(NULL));
  struct rooms* my_room[7];
  int room_cnt = 10, i;
  int rand_room_num = rand()%room_cnt;
  int start_room = rand()%7;
  int end_room = rand()%7;
  while (end_room == start_room) end_room = rand()%7;
  char room_name_list[10][9], temp[8];
  strcpy(room_name_list[0], "Azeroth");
  strcpy(room_name_list[1], "Westfall");
  strcpy(room_name_list[2], "Duskwood");
  strcpy(room_name_list[3], "Kalidar");
  strcpy(room_name_list[4], "Felwood");
  strcpy(room_name_list[5], "Mulgore");
  strcpy(room_name_list[6], "Wetlands");
  strcpy(room_name_list[7], "Icecrown");
  strcpy(room_name_list[8], "Sunwell");
  strcpy(room_name_list[9], "Elfgate");

  for(i = 0; i < 7; i++){
    my_room[i] = buildroom();

    //assign room name
    strcpy(my_room[i]->room_name, room_name_list[rand_room_num]);
    strcpy(temp, room_name_list[rand_room_num]);
    strcpy(room_name_list[rand_room_num], room_name_list[--room_cnt]);
    strcpy(room_name_list[room_cnt], temp);
    rand_room_num = rand()%room_cnt;

    //assign room type
    if (i == start_room)
      strcpy(my_room[i]->room_type, "START_ROOM");
    else if (i == end_room)
      strcpy(my_room[i]->room_type, "END_ROOM");
    else
      strcpy(my_room[i]->room_type, "MID_ROOM");
  }

  // Create all connections in graph
  while (IsGraphFull(my_room) == 0)
  {
    AddRandomConnection(my_room);
  }
  write_file(my_room);

  for(i = 0; i < 7; i++){
    // printf("Room %d: %s\n", (i+1), my_room[i]->room_name);
    // int num_con = 0;
    // for (num_con = 0; num_con < my_room[i]->num_connection; num_con++) {
    //   printf("Connection %d: %s\n", num_con+1, my_room[i]->connections[num_con]);
    // }
    // printf("Type  : %s\n\n", my_room[i]->room_type);
    free_rooms(my_room[i]);
  }
  return 0;
}


/*
Functi Name: buildroom.
Description: create struct pointer and malloc the memory then initialization.
*/
struct rooms* buildroom(){
  struct rooms* A = malloc(sizeof(struct rooms));
  int i = 0;
  for (i = 0; i < 6; i ++){
    memset(A->connections[i], '\0' ,9);
  }
  A->num_connection = 0;
  memset(A->room_name, '\0' ,9);
  memset(A->room_type, '\0' ,11);
  return A;
}


/*
Functi Name: free_rooms.
Description: Free the memory set to the rooms.
*/
void free_rooms(struct rooms* A){
  free(A);
}


/*
Functi Name: IsSameRoom.
Description: Returns true if Rooms x and y are the same Room, false otherwise.
*/
int IsSameRoom(struct rooms* x, struct rooms* y)
{
  if (!strcmp(x->room_name, y->room_name)) return 1;
  else return 0;
}


/*
Functi Name: ConnectRoom.
Description: Connects Rooms x and y together, does not check if this connection is valid
*/
void ConnectRoom(struct rooms* x, struct rooms* y)
{
  int i = x->num_connection;
  int j = y->num_connection;

  strcpy(x->connections[i], y->room_name);
  x->num_connection++;
  strcpy(y->connections[j], x->room_name);
  y->num_connection++;
}


/*
Functi Name: ConnectionAlreadyExists.
Description: Returns true if a connection from Room x to Room y already exists, false otherwise
*/
int ConnectionAlreadyExists(struct rooms* A, struct rooms* B)
{
  int A_has_B = 0, B_has_A = 0, i = 0;
  for (i = 0; i < 7; i++){
    A_has_B = (!strcmp(B->room_name, A->connections[i]) || A_has_B);
    B_has_A = (!strcmp(A->room_name, B->connections[i]) || B_has_A);
  }
  if (A_has_B && B_has_A) return 1;
  else return 0;
}


/*
Functi Name: CanAddConnectionFrom.
Description: Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
*/
int CanAddConnectionFrom(struct rooms* A)
{
  if (A->num_connection <= 6) return 1;
  else return 0;
}


/*
Functi Name: GetRandomRoom.
Description: Returns a random Room, does NOT validate if connection can be added
*/
struct rooms* GetRandomRoom(struct rooms* my_room[])
{
  srand((unsigned)time(NULL));
  int rand_room_num = rand()%7;
  return my_room[rand_room_num];
}


/*
Functi Name: GetRandomRoom.
Description: Returns true if all rooms have 3 to 6 outbound connections, false otherwise
*/
int IsGraphFull(struct rooms* my_room[])
{
  int i = 0, Full = 1;
  for (i = 0; i < 7; i++) {
    if(my_room[i]->num_connection <= 6 && my_room[i]->num_connection >= 3) Full = Full && 1;
    else Full = 0;
  }
  return Full;
}


/*
Functi Name: GetRandomRoom.
Description: Adds a random, valid outbound connection from a Room to another Room
*/
void AddRandomConnection(struct rooms* my_room[])
{
  struct rooms* A = GetRandomRoom(my_room);
  struct rooms* B = GetRandomRoom(my_room);
  while(1)
  {
    A = GetRandomRoom(my_room);

    if (CanAddConnectionFrom(A) == 1)
      break;
  }

  do
  {
    B = GetRandomRoom(my_room);
  }
  while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);

  ConnectRoom(A, B);
}


/*
Functi Name: GetRandomRoom.
Description: Adds a random, valid outbound connection from a Room to another Room
*/
void write_file(struct rooms* my_room[]){
  int proccess_id = getpid();
  char id[10];
  char dir_name[20] = "biz.rooms.";
  sprintf(id, "%d", proccess_id);
  strcat(dir_name, id);
  mkdir(dir_name, S_IRWXU | S_IRWXG);
  chdir(dir_name);

  int i = 0, j = 0;
  for (i = 0; i < 7; i++){
    char file_name[20];
    memset(file_name, '\0', 20);
    strcpy(file_name, my_room[i]->room_name);
    strcat(file_name, "_room");
    FILE *output;
    output = fopen(file_name, "w");
    fprintf(output, "ROOM NAME: %s\n", my_room[i]->room_name);
    for (j = 0; j < my_room[i]->num_connection; j++) {
      fprintf(output, "CONNECTION %d: %s\n", j+1, my_room[i]->connections[j]);
    }
    fprintf(output, "ROOM TYPE: %s\n", my_room[i]->room_type);
    fclose(output);
  }
}
