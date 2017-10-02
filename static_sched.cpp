#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <pthread.h>

#define DIVISIBLE 1
#define NONDIVISIBLE 0

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

pthread_mutex_t sum_protect = PTHREAD_MUTEX_INITIALIZER;

float sum = 0.0;

typedef struct {
  int functionid;
  float a;
  float b;
  unsigned long n;
  int intensity;
  int start;
  int length;

} integrateArgs;

void *integrationThreadLevel (integrateArgs *args) {
  int i;
  float x = 0.0;
  float *result = (float *) malloc (sizeof(float));
  *result = 0.0;


  float multiplier = (args->b - args->a)/ (float) args->n;
  int end = args->start + args->length;

  for (i=args->start; i<end; i++) {
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
  return (void *)result;
}

void integrationIterationLevel (integrateArgs *args) {

  int i;
  float x = 0.0;

  float multiplier = (args->b - args->a)/ (float) args->n;
  int end = args->start + args->length;

  for (i=args->start; i<end; i++) {
    x = args->a + ((float)i + 0.5) * multiplier;


    switch (args->functionid) {
      case 1:
        pthread_mutex_lock(&sum_protect);
        sum = sum + f1(x, args->intensity)*multiplier;
        pthread_mutex_unlock(&sum_protect);
        break;
      case 2:
        pthread_mutex_lock(&sum_protect);
        sum = sum + f2(x, args->intensity)*multiplier;
        pthread_mutex_unlock(&sum_protect);
        break;
      case 3:
        pthread_mutex_lock(&sum_protect);
        sum = sum + f3(x, args->intensity)*multiplier;
        pthread_mutex_unlock(&sum_protect);
        break;
      case 4:
        pthread_mutex_lock(&sum_protect);
        sum = sum + f4(x, args->intensity)*multiplier;
        pthread_mutex_unlock(&sum_protect);
        break;
      default:
        break;
    }
  }
}


int main (int argc, char* argv[]) {

  if (argc < 8) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<std::endl;
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

  float result = 0.0;
  int k;
  int divisibility = DIVISIBLE;


  /*check for wrong input*/
  if (functionid < 0 || functionid > 4) {
    std::cerr<<"ERROR: Enter a function id between 1 and 4 "<<std::endl;
    exit(-1);
  }
  else {
    pthread_t *threads;
    integrateArgs *args;
    int chunksize = n/nbthreads;
    if (n%nbthreads != 0) {
      divisibility = NONDIVISIBLE;
    }

    threads = (pthread_t *) malloc(nbthreads * sizeof(pthread_t));
    args = (integrateArgs *) malloc(nbthreads * sizeof(integrateArgs));

    if (threads == NULL || args == NULL){
      std::cout<<"Unable to allocate memory";
      exit(-1);
    }
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    for (k=0; k<nbthreads; k++)
    {
      args[k].functionid = functionid;
      args[k].a = a;
      args[k].b = b;
      args[k].n = n;
      args[k].intensity = intensity;
      args[k].start = k*chunksize;
      args[k].length = chunksize;
      if(divisibility == NONDIVISIBLE && k==nbthreads-1)
        args[k].length = n - (nbthreads-1)*chunksize;

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
      else {
        std::cout<<"Please enter either thread or iteration!!"<<std::endl;
        exit(-1);
      }


    }
    for (k=0; k<nbthreads; k++){
      if (synctype.compare("iteration")==0){
        pthread_join(threads[k], NULL);
      }
      else if (synctype.compare("thread")==0){

        void *retval;
        pthread_join(threads[k], &retval);
        result = result + *(float *)retval;
      }

    }
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;

    if (synctype.compare("iteration")==0){
    std::cout<<sum<<std::endl;
  }
  else{
    std::cout<<result<<std::endl;
  }
  std::cerr<<elapsed_seconds.count()<<std::endl;
    free (threads);
    free (args);


}


  return 0;
}
