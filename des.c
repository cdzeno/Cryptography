#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "string.h"

typedef struct _L0R0
{
  unsigned int l0: 6;
  unsigned int r0: 6;
} L0R0;

typedef struct _Key
{
  unsigned int key: 9;
} Key;

uint8_t s1 [2][8] = {{5,2,1,6,3,4,7,0},{1,4,6,2,0,7,5,3}};
uint8_t s2 [2][8] = {{4,0,6,5,7,1,3,2},{5,3,0,7,6,2,1,4}};

uint8_t expansion (uint8_t r0); //in: 6bit, out: 8bit
uint8_t getBox (uint8_t sbox[2][8], uint8_t in_sbox); //in: mat3bit 4bit, out: 3bit
uint8_t feistel (uint8_t r0, uint8_t key); //in: 6bit 8bit, out: 6bit
L0R0 turn (L0R0 input, uint8_t key); //in: L0R0 8bit, out: L0R0
uint8_t keySchedule (Key key, uint8_t n_round);//in: 9bit 4bit, out: 8bit
unsigned int bin_string_to_int (char* s); //in: string binary digit out: unsigned int
void des (L0R0 plaintext, Key key, uint64_t n_round);

uint8_t expansion (uint8_t r0)
{
  return ( (r0 <<2) & 0b11000000 ) |
	 ( ((r0 & 0b00001000) >> 1 | (r0 & 0b00001000) << 1) | ((r0 & 0b00000100) << 1 | (r0 & 0b00000100) << 3) ) |
	 ( r0 & 0b00000011 );
}

uint8_t getBox (uint8_t sbox[2][8], uint8_t in_sbox)
{
  uint8_t row = (in_sbox & 0b00001000) >> 3;
  uint8_t col = in_sbox & 0b00000111;
  return sbox[row][col];
}

uint8_t feistel (uint8_t r0, uint8_t key)
{
  uint8_t r0_xor_key = expansion(r0) ^ key;
  uint8_t out_s1 = getBox(s1, (r0_xor_key & 0b11110000) >> 4);
  uint8_t out_s2 = getBox(s2, r0_xor_key & 0b00001111);
  return ((out_s1 << 3) | out_s2);
}

L0R0 turn (L0R0 input, uint8_t key)
{
  L0R0 output;
  output.l0 =  input.r0;
  output.r0 =  feistel(input.r0, key) ^ input.l0;
  return output;
}

uint8_t keySchedule (Key key, uint8_t n_round)
{
  //n_round %= 9; //fare in des e prendo uint16_t (0-8 shift)
  Key bitmask;
  bitmask.key = (((uint16_t) pow(n_round, 2.0) - 1) << (uint16_t)(9 - n_round));
  return ( (key.key << n_round) | ((key.key & bitmask.key) >> (uint16_t)(9 - n_round)) );
}

void des (L0R0 plaintext, Key key, uint64_t n_round)
{
  L0R0 input = plaintext;

  uint64_t i;
  for(i=0; i<n_round; i++)
  {
    uint8_t roundkey = keySchedule(key, (i%9));
    //printf("round_key %d: 0x%x \n", (int)i+1, roundkey);
    input = turn(input,roundkey);
    //printf("output %d: left=0x%x  right=0x%x\n", (int)i+1, input.l0,input.r0);
  }
  printf("Ciphertext: left=0x%x  right=0x%x\n", input.l0, input.r0);

}

unsigned int bin_string_to_int (char* s){

  unsigned int binary = 0;
  unsigned int index = strlen(s) - 1;

  while(*s != '\0'){
        binary += (*s - 48)*pow((double)2, (double)index);
        index--;
        s++;
  }
  return binary;
}

int main(int argc, char* argv[])
{
  // argv[1] = 12 bit plaintext
  // argv[2] = 9 bit key
  assert(argc == 3);
  assert((int)strlen(argv[1]) == 12 && (int)strlen(argv[2]) == 9);

  char l0[7] = {'0','0','0','0','0','0','\0'};
  char r0[7] = {'0','0','0','0','0','0','\0'};
  char key[10] = {'0','0','0','0','0','0','0','0','0','\0'};
  (void)strncpy(l0, argv[1], 6);
  (void)strncpy(r0, argv[1]+6, 6);
  (void)strncpy(key, argv[2], 9);

  printf("Plaintext: l0=0b%s | r0=0b%s\nKey: 0b%s\n", l0, r0, key);

  L0R0 plain = {.l0 = bin_string_to_int(l0), .r0 = bin_string_to_int(r0)};
  Key key_plain = {.key = bin_string_to_int(key)};

  des(plain, key_plain, 3);
  return 0;
}
