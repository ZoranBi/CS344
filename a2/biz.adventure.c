/********************************************************
Name: biz.adventure.c
Author: Zongzhe Bi
Date: 10/18/2018
********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

struct rooms {
   char room_name[256];
   char room_type[256];
   int num_connection;
   char connections[6][256];
};

struct steps{
   int num_steps;
   char path[30][30];
};
/*****************************************************************
Protocol Parts
*/
struct rooms* buildroom();
void free_rooms(struct rooms*);
char* read_data();
void fill_room(char*, struct rooms**);
void assign_name(struct rooms**, char*, int);
void assign_connections(struct rooms**, char*, int);
void assign_type(struct rooms**, char*, int);
void print_room_info(struct rooms**, int);
void player_interface(struct rooms**, int, struct steps*);
void take_input(struct rooms**, int, struct steps*);
int find_start_room(struct rooms**);
int move_location(struct rooms**, char*);
void add_step(struct steps*, char*);
void* start_time(void*);
void* print_time();
/****************************************************************/


/*****************************************************************
main function
*/
int main(int argc, char const *argv[]) {
  pthread_mutex_lock(&my_mutex);
  char* ret_result;
  int i = 0, j, room_num = 0;
  struct rooms* my_room[7];
  struct steps* my_path = malloc(sizeof(struct steps));
  my_path->num_steps = 0;
  for (i = 0; i < 30; i ++){
    memset(my_path->path[i], '\0' ,30);
  }

  for(i = 0; i < 7; i++){
    my_room[i] = buildroom();
  }

  //Read room data from room files
  ret_result = read_data();
  fill_room(ret_result, my_room);

  //Start Game
  room_num = find_start_room(my_room);
  if(room_num == 10){
    printf("ERROR: Cannot find the START_ROOM\n");
    exit(1);
  }
  player_interface(my_room, room_num, my_path);

  free(ret_result);
  free(my_path);
  for (i = 0; i < 7; i++) {
    free_rooms(my_room[i]);
  }
  return 0;
}


/*
Functi Name: end_game.
Description: Print information when game end.
*/
void end_game(struct steps* my_path){
  int num_step = my_path->num_steps;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_step);
  int i = 0;
  for(i = 0; i <= num_step; i ++){
    printf("%s\n", my_path->path[i]);
  }
}


/*
Functi Name: is_win.
Description: Judge if game is end. Return 1 for True; 0 for False.
*/
int is_win(struct rooms* my_room[], int room_num){
  if(!strcmp(my_room[room_num]->room_type, "END_ROOM")) return 1;
  else return 0;
}


/*
Functi Name: move_location.
Description: Move current location to next. return array number of destination
*/
int move_location(struct rooms* my_room[], char* destination){
  int room_num = 0;
  for (room_num = 0; room_num < 7; room_num ++){
    if(!strcmp(my_room[room_num]->room_name, destination)) return room_num;
  }
  return 10;
}


/*
Functi Name: find_start_room.
Description: Find out start room. return array number
*/
int find_start_room(struct rooms * my_room[]){
  int room_num = 0;
  for (room_num = 0; room_num < 7; room_num ++){
    if(!strcmp(my_room[room_num]->room_type, "START_ROOM")) return room_num;
  }
  return 10;
}


/*
Functi Name: add_step.
Description: Steps record.
*/
void add_step(struct steps* my_path, char* input){
  int num_step = my_path->num_steps;
  my_path->num_steps++;
  strcpy(my_path->path[num_step], input);
}


/*
Functi Name: take_input.
Description: Take player's input from keyboard. Response for different enter.
*/
void take_input(struct rooms* my_room[], int room_num, struct steps* my_path){
  pthread_t timeThread;
  int i = 0, valid = 0;
  char play_input[256];
  memset(play_input, '\0', 256);
  scanf("%s", play_input);
  for(i = 0; i < my_room[room_num]->num_connection; i ++){
   if (!strcmp(my_room[room_num]->connections[i], play_input)){
     valid = 1 || valid;
     room_num = i;
   }
  }

  if (valid){
    room_num = move_location(my_room, play_input);
    add_step(my_path, play_input);
    if(is_win(my_room, room_num)) end_game(my_path);
    else player_interface(my_room, room_num, my_path);
  }
  else if (!strcmp("time", play_input)){
    pthread_mutex_unlock(&my_mutex);
    pthread_create(&timeThread, NULL, start_time, &my_mutex);
    pthread_join(timeThread, NULL);
    pthread_mutex_lock(&my_mutex);
    print_time();
    player_interface(my_room, room_num, my_path);
  }
  else{
    printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
    player_interface(my_room, room_num, my_path);
  }
}


/*
Functi Name: player_interface.
Description: player interface. Show information to player.
*/
void player_interface(struct rooms * my_room[], int room_num, struct steps* my_path){
   int i = 0;
   printf("\nCURRENT LOCATION: %s\n", my_room[room_num]->room_name);
   printf("POSSIBLE CONNECTIONS: ");
   for (i = 0; i < my_room[room_num]->num_connection; i++) {
      printf("%s ", my_room[room_num]->connections[i]);
   }
   printf("\nWHERE TO? >");
   take_input(my_room, room_num, my_path);
}

/*
Functi Name: buildroom.
Description: create struct pointer and malloc the memory then initialization.
*/
struct rooms* buildroom(){
  struct rooms* A = malloc(sizeof(struct rooms));
  int i = 0;
  for (i = 0; i < 6; i ++){
    memset(A->connections[i], '\0' ,256);
  }
  A->num_connection = 0;
  memset(A->room_name, '\0' ,256);
  memset(A->room_type, '\0' ,256);
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
Functi Name: read_data.
Description: read data from target files.
*/
char* read_data(){
  int st_mtime_result = 0;
  char tar_keywords[32] = "biz.rooms.";
  char* tar_file;
  tar_file = malloc(sizeof(char[32]));
  memset(tar_file, '\0', 32);

  DIR* dp = opendir(".");
  struct dirent *entry;
  struct stat statbuf;

  if(dp > 0){
    while ((entry = readdir(dp)) != NULL) {
      if (strstr(entry->d_name, tar_keywords) != NULL) {
        stat(entry->d_name, &statbuf);
        if ((int)statbuf.st_mtime > st_mtime_result) {
          st_mtime_result = (int)statbuf.st_mtime;
          strcpy(tar_file, entry->d_name);
        }
      }
    }
  }
  else
    printf("Error: Cannot open file\n");

  closedir(dp);
  return tar_file;
}

void fill_room(char *dir, struct rooms* my_room[]){
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
  char buffer[256];
  FILE *inputs;
  int cnt = 0, i = 0;

	if ((dp = opendir(dir)) == NULL) {
		fprintf(stderr, "Can't open directory %s\n", dir);
		return ;
	}
	chdir(dir);
	while ((entry = readdir(dp)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 )
				continue;
    stat(entry->d_name, &statbuf);

    inputs = fopen(entry->d_name, "r");
    if(inputs == NULL) {
      printf("ERROR: Cannot open the file\n");
    }
    //fill info to rooms
    fgets(buffer, 256, inputs);
    assign_name(my_room, buffer, cnt);
    for (i = 0; i < 7; i ++){
      fgets(buffer, 256, inputs);
      if (buffer[0] == 'C'){
         assign_connections(my_room, buffer, cnt);
      }
      else if (buffer[0] == 'R') assign_type(my_room, buffer, cnt);
    }
    cnt++;
	}
	chdir("..");
	closedir(dp);
}

void assign_name(struct rooms *my_room[], char* buffer, int cnt){
  int i = 0;
  char temp[30];
  for(i = 0; i < 30; i ++){
    if(buffer[i + 11] == '\n' || buffer[i + 11] == '\0') break;
    else temp[i] = buffer[i + 11];
  }
  strcpy(my_room[cnt]->room_name, temp);
}

void assign_connections(struct rooms *my_room[], char* buffer, int cnt){
   int i = 0, num_connection = 0;
   char temp[30];
   num_connection = my_room[cnt]->num_connection;
   for(i = 0; i < 30; i ++){
     if(buffer[i + 14] == '\n' || buffer[i + 14] == '\0') break;
     else temp[i] = buffer[i + 14];
   }
   my_room[cnt]->num_connection++;
   strcpy(my_room[cnt]->connections[num_connection], temp);
}

void assign_type(struct rooms *my_room[], char* buffer, int cnt){
   int i = 0;
   char temp[30];
   for(i = 0; i < 30; i ++){
     if(buffer[i + 11] == '\n' || buffer[i + 11] == '\0') break;
     else temp[i] = buffer[i + 11];
   }
   strcpy(my_room[cnt]->room_type, temp);
}

void print_room_info(struct rooms *my_room[], int room_num){
   int i = 0, j = 0;
   if (room_num == -1){
     for(i = 0; i < 7; i++){
       printf("ROOM NAME: %s\n", my_room[i]->room_name);
       for (j = 0; j < my_room[i]->num_connection; j++) {
          printf("CONNECTION %d: %s\n", j+1, my_room[i]->connections[j]);
       }
       printf("ROOM TYPE: %s\n\n", my_room[i]->room_type);
     }
   }
   else{
     printf("ROOM NAME: %s\n", my_room[room_num]->room_name);
     for (j = 0; j < my_room[room_num]->num_connection; j++) {
        printf("CONNECTION %d: %s\n", j+1, my_room[room_num]->connections[j]);
     }
     printf("ROOM TYPE: %s\n\n", my_room[room_num]->room_type);
   }
}


/*****************************************************************
thread time related part
*/
void* start_time(void *my_mutex){
  FILE* time_file = fopen("currentTime.txt", "w");
  if (time_file == NULL)
  {
      perror("time_file set wrong");
  }
  time_t current_time;
  char* time_str;
  //get time
  current_time = time(0);
  time_str = ctime(&current_time);
  //write time in file again.
  fputs(time_str, time_file);
  fclose(time_file);
}

void* print_time(){
  FILE* time_file = fopen("currentTime.txt", "r");
  if ( NULL == time_file )
      {
          perror("time_file get wrong");
      }
  char times[512];
  fgets(times, sizeof(times), time_file);
  printf("\n%s\n", times);
  fclose(time_file);
}
