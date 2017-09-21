#include <iostream>
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

void integrationIterationLevel (integrateArgs *args) {

  int i;
  float x = 0.0;

  float multiplier = (args->b - args->a)/ (float) args->n;
  int end = args->start + args->length;

  for (i=args->start; i<end; i++) {
    x = args->a + ((float)i + 0.5) * multiplier;


    pthread_mutex_lock(&sum_protect);
    switch (args->functionid) {
      case 1:
        sum = sum + f1(x, args->intensity)*multiplier;
        break;
      case 2:
        sum = sum + f2(x, args->intensity)*multiplier;
        break;
      case 3:
        sum = sum + f3(x, args->intensity)*multiplier;
        break;
      case 4:
        sum = sum + f4(x, args->intensity)*multiplier;
        break;
      default:
      break;
    }
    pthread_mutex_unlock(&sum_protect);
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

  float result;
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

      int err = pthread_create(&threads[k],NULL,
        (void *(*)(void *)) integrationIterationLevel,
        (void *) &args[k]);
      if (err!=0)
          std::cout<<"ERROR creating thread!!!!"<<std::endl;


    }
    for (k=0; k<nbthreads; k++)
    {
      pthread_join(threads[k], NULL);

    }

    std::cout<<sum<<std::endl;

    free (threads);
    free (args);


}


  return 0;
}
