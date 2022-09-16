#include <stdint.h>
#include <iostream>

#define WIDTH0 4
#define HEIGHT0 3

using namespace std;

extern "C" uint16_t EightBitHistogram(uint16_t width, uint16_t height, const uint8_t* startAddress, uint16_t* startHistogram); //Extern function written in Assembly


const uint8_t image0 [HEIGHT0][WIDTH0] =   //Caso de teste 1
{
  {20, 16, 16, 18},
  {255, 255, 0, 0},
  {32, 32, 32, 32}
};

void showHistogram(uint16_t* startAddress)
{
  int j;
  uint16_t *histogramArray = startAddress;

  cout << "**********Histograma final************\n";
  
  for (j = 0; j < 256; j++)
  {
    cout << j << ":" << histogramArray[j] << "\n";    
  }
}
int main (void)
{

int pixelsProcessados = 0;

//const uint8_t *startAddress = &image0[0][0];     //Get address of the first element

uint16_t histograma[256];

pixelsProcessados = EightBitHistogram(WIDTH0, HEIGHT0, *image0, histograma);

showHistogram(histograma);

return 0;

}