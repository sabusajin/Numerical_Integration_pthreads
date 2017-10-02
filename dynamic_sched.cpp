#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

typedef struct {
  int functionid;
  float a;
  float b;
  unsigned long n;
  int intensity;

} integrateArgs;

float iteration_sum = 0.0;
float chunk_sum = 0.0;


int granularity = 0;
unsigned long  commence = 0, stop = 0;
pthread_mutex_t getNext_protect = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sum_protect = PTHREAD_MUTEX_INITIALIZER;

bool done() {
  if (commence == stop)
    return true;
  else
    return false;
}
void getNext(int *begin, int *end) {
  pthread_mutex_lock(&getNext_protect);
  *begin = commence;
  if (commence + granularity >= stop) {
    *end = stop;
    commence = stop;
  }
  else {
    *end = commence + granularity;
    commence = commence + granularity;
  }
  pthread_mutex_unlock(&getNext_protect);
}

void *integrationThreadLevel (integrateArgs *args) {

  float x = 0.0;
  float *result = (float *) malloc (sizeof(float));
  *result = 0.0;
  float multiplier = (args->b - args->a)/ (float) args->n;

  while (!done()) {
    int begin, end;
    getNext (&begin, &end);
    int temp_sum;
    temp_sum = 0;
    for (int i=begin; i<end; i++)
    {
      x = args->a + ((float)i + 0.5) * multiplier;
      switch (args->functionid) {
        case 1:

          *result = *result + f1(x, args->intensity)*multiplier;

          break;
        case 2:

          *result = *result + f2(x, args->intensity)*multiplier;

          break;
        case 3:

          *result = *result + f3(x, args->intensity)*multiplier;

          break;
        case 4:

          *result = *result + f4(x, args->intensity)*multiplier;

          break;
        default:
          break;
      }

    }

  }
  return (void *)result;
}

void integrationIterationLevel(integrateArgs *args) {

  float x = 0.0;

  float multiplier = (args->b - args->a)/ (float) args->n;

  while (!done()) {
    int begin, end;
    getNext (&begin, &end);
    for (int i=begin; i<end; i++)
    {
      x = args->a + ((float)i + 0.5) * multiplier;
      switch (args->functionid) {
        case 1:
          pthread_mutex_lock(&sum_protect);
          iteration_sum = iteration_sum + f1(x, args->intensity)*multiplier;
          pthread_mutex_unlock(&sum_protect);
          break;
        case 2:
          pthread_mutex_lock(&sum_protect);
          iteration_sum = iteration_sum + f2(x, args->intensity)*multiplier;
          pthread_mutex_unlock(&sum_protect);
          break;
        case 3:
          pthread_mutex_lock(&sum_protect);
          iteration_sum = iteration_sum + f3(x, args->intensity)*multiplier;
          pthread_mutex_unlock(&sum_protect);
          break;
        case 4:
          pthread_mutex_lock(&sum_protect);
          iteration_sum = iteration_sum + f4(x, args->intensity)*multiplier;
          pthread_mutex_unlock(&sum_protect);
          break;
        default:
          break;
      }

    }
  }

}
void integrationChunkLevel(integrateArgs *args) {

  float x = 0.0;

  float multiplier = (args->b - args->a)/ (float) args->n;

  while (!done()) {
    int begin, end;
    getNext (&begin, &end);
    float temp_sum;
    temp_sum = 0.0;
    for (int i=begin; i<end; i++)
    {
      x = args->a + ((float)i + 0.5) * multiplier;
      switch (args->functionid) {
        case 1:

          temp_sum = temp_sum + f1(x, args->intensity)*multiplier;

          break;
        case 2:

          temp_sum = temp_sum + f2(x, args->intensity)*multiplier;

          break;
        case 3:

          temp_sum = temp_sum + f3(x, args->intensity)*multiplier;

          break;
        case 4:

          temp_sum = temp_sum + f4(x, args->intensity)*multiplier;

          break;
        default:
          break;
      }

    }
    pthread_mutex_lock(&sum_protect);
    chunk_sum = chunk_sum + temp_sum;
    pthread_mutex_unlock(&sum_protect);

  }

}

int main (int argc, char* argv[]) {

  if (argc < 9) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync> <granularity>"<<std::endl;
    return -1;
  }
  /*local variable declarations*/
  int functionid = atoi(argv[1]);
  float a = atof(argv[2]);
  float b = atof(argv[3]);
  unsigned long n = atol(argv[4]);
  int intensity = atoi(argv[5]);
  int nbthreads = atoi(argv[6]);
  std::string synctype = argv[7];
  granularity = atoi(argv[8]);
  stop = n;
  int k = 0;

  float thread_result = 0.0;


  /*check for wrong input*/
  if (functionid < 0 || functionid > 4) {
    std::cerr<<"ERROR: Enter a function id between 1 and 4 "<<std::endl;
    exit(-1);
  }
  else {
    pthread_t *threads;
    integrateArgs *args;
    threads = (pthread_t *) malloc(nbthreads * sizeof(pthread_t));
    args = (integrateArgs *) malloc(nbthreads * sizeof(integrateArgs));

    if (threads == NULL || args == NULL){
      std::cout<<"Unable to allocate memory";
      exit(-1);
    }
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    for (k=0; k<nbthreads; k++){

      args[k].functionid = functionid;
      args[k].a = a;
      args[k].b = b;
      args[k].n = n;
      args[k].intensity = intensity;

      if (synctype.compare("iteration")==0){
        int err = pthread_create(&threads[k],NULL,
          (void *(*)(void *)) integrationIterationLevel,
          (void *) &args[k]);
        if (err!=0)
            std::cout<<"ERROR creating thread!!!!"<<std::endl;
      }
      else if (synctype.compare("thread")==0){
        int err = pthread_create(&threads[k],NULL,
           (void *(*)(void *)) integrationThreadLevel,
          (void *) &args[k]);
        if (err!=0)
            std::cout<<"ERROR creating thread!!!!"<<std::endl;
      }
      else if (synctype.compare("chunk")==0){
        int err = pthread_create(&threads[k],NULL,
          (void *(*)(void *)) integrationChunkLevel,
          (void *) &args[k]);
        if (err!=0)
            std::cout<<"ERROR creating thread!!!!"<<std::endl;
      }
      else {
        std::cout<<"Please enter either thread or iteration or chunk for sync!!"<<std::endl;
        exit(-1);
      }


    }
    for (k=0; k<nbthreads; k++){
      if (synctype.compare("iteration")==0 || synctype.compare("chunk")==0){
        pthread_join(threads[k], NULL);
      }
      else if (synctype.compare("thread")==0) {
        void *retval;
        pthread_join(threads[k], &retval);
        thread_result = thread_result + *(float *)retval;
      }
    }
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    if (synctype.compare("iteration")==0){
      std::cout<<iteration_sum<<std::endl;
    }
    else if (synctype.compare("chunk")==0){
      std::cout <<chunk_sum<<std::endl;

    }
    else if (synctype.compare("thread")==0){
      std::cout <<thread_result<<std::endl;
    }
    std::cerr<<elapsed_seconds.count()<<std::endl;

    free (threads);
    free (args);

  }


  return 0;
}
