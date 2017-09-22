#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>


float sequential_integrate (int argc, char* argv[]);

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

int main (int argc, char* argv[]) {

  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  float integrate = sequential_integrate(argc, argv);

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;

  std::cout<<integrate<<std::endl;

  std::cerr<<elapsed_seconds.count()<<std::endl;

  return 0;
}

float sequential_integrate (int argc, char* argv[]) {

  int functionid = atoi(argv[1]);
  float a = atof(argv[2]);
  float b = atof(argv[3]);
  unsigned long n = atol(argv[4]);
  uint32_t intensity = atoi(argv[5]);

  float result = 0.0, x = 0.0;
  float multiplier = (b-a)/(float)n;
  int i;

  for (i=0; i<n; i++) {

    x = a + ((float)i + 0.5) * multiplier;

    switch (functionid) {

      case 1:
        result = result + f1(x, intensity)*multiplier;
        break;
      case 2:
        result = result + f2(x, intensity)*multiplier;
        break;
      case 3:
        result = result + f3(x, intensity)*multiplier;
        break;
      case 4:
        result = result + f4(x, intensity)*multiplier;
        break;
      default:
        break;
    }

  }
  return result;
}
