
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex ;

// void* print_time()
// {
//   FILE* time_file = fopen("currentTime.txt", "r");
//   if ( NULL == time_file )
//       {
//           perror("time_file get wrong");
//       }
//   char times[512];
//   fgets(times, sizeof(times), time_file);
//   printf("\n%s\n", times);
//   fclose(time_file);
// }
//
// void* start_time(void* mutex)
// {
//   FILE* time_file = fopen("currentTime.txt", "w");
//   if (time_file == NULL)
//   {
//       perror("time_file set wrong");
//   }
//   time_t current_time;
//   char* time_str;
//   //get time.
//   current_time = time(0);
//   time_str = ctime(&current_time);
//   //write time in file again.
//   fputs(time_str, time_file);
//   fclose(time_file);
// }

void *print_msg(void *arg){
        int i=0;
        pthread_mutex_lock(&mutex);
        for(i=0;i<15;i++){
                printf("output : %d\n",i);
                usleep(100);
        }
        pthread_mutex_unlock(&mutex);
}

int main(int argc,char** argv){
        pthread_t id1;
        pthread_t id2;
        pthread_mutex_init(&mutex,NULL);
        pthread_create(&id1,NULL,print_msg,NULL);
        pthread_create(&id2,NULL,print_msg,NULL);
        pthread_join(id1,NULL);
        pthread_join(id2,NULL);
        pthread_mutex_destroy(&mutex);
        return 1;
}
