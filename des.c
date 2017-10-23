#include <stdio.h>
#include <stdlib.h>

typedef struct _L0R0 {
  unsigned int l0: 6;
  unsigned int r0: 6;
} L0R0;

typedef struct _Plain {
  unsigned int l0: 6;
  unsigned int r0: 6;
} Plain;

typedef struct _Key {
  unsigned int key:9;
} Key;

typedef struct _Bitmask {
  unsigned int bitmask:9;
} Bitmask;

unsigned char s1 [2][8] = {{'5','2','1','6','3','4','7','0'},{'1','4','6','2','0','7','5','3'}};
unsigned char s2 [2][8] = {{'4','0','6','5','7','1','3','2'},{'5','3','0','7','6','2','1','4'}};

unsigned char expansion (unsigned char r0);
unsigned char getbox (unsigned char sbox[2][8], unsigned char in_sbox);
unsigned char feistel (unsigned char r0, unsigned char key);
L0R0 turn (L0R0 input, unsigned char key);

unsigned char key_schedule (Key key, int n_round);
char* des (char* plaintext, char* key);

int main(int argc, char const *argv[]) {

  printf("r0_xor_key: 0x%x\n", expansion((unsigned char)0x011011) ^ ((unsigned char)0x01001101));
  return 0;
}

unsigned char expansion (unsigned char r0){
  // OK
  unsigned char tail = r0 & 0b00000011;
  unsigned char head = (r0 <<2) & 0b11000000;
  unsigned char body = ((r0 & 0b00001000) >> 1 | (r0 & 0b00001000) << 1) | ((r0 & 0b00000100) << 1 | (r0 & 0b00000100) << 3);
  return (head | body | tail);
}

unsigned char getbox (unsigned char sbox[2][8], unsigned char in_sbox){
  // OK
  unsigned int row = (unsigned int) ((in_sbox & 0b00001000) >> 3);
  unsigned int col = (unsigned int) (in_sbox & 0b00000111);

  return sbox[row][col];
}

unsigned char feistel (unsigned char r0, unsigned char key){
  unsigned char r0_xor_key = expansion(r0) ^ key; // <--- Debugging HERE!!
  unsigned char in_s1 = r0_xor_key & 0b00001111;
  unsigned char in_s2 = r0_xor_key & 0b11110000;

  // "- 0x30"  => serve per convertire char della sbox in numero
  unsigned char out_s1 = getbox(s1, in_s1) - 0x30;
  unsigned char out_s2 = getbox(s2, in_s2) - 0x30;

  return ((out_s1 << 3) | out_s2);
}

L0R0 turn (L0R0 input, unsigned char key){
  unsigned char out_l1 =  input.l0;
  unsigned char out_r1 =  feistel(input.r0, key) ^ input.l0;
  L0R0 output = { .l0 = out_l1 , .r0 = out_r1};
  return output;
}

unsigned char key_schedule (Key key, int n_round){
  Bitmask bit_mask;
  int right_shift = 9 - n_round - 1;
  n_round = n_round - 1;


  switch (n_round) {
    case 1:
      bit_mask.bitmask = 0b100000000;
      break;
    case 2:
      bit_mask.bitmask = 0b110000000;
      break;
    case 3:
      bit_mask.bitmask = 0b111000000;
      break;
    case 4:
      bit_mask.bitmask = 0b111100000;
      break;
  }

  return (((key.key << n_round) | ((key.key & bit_mask.bitmask) >> right_shift)) >> 1);
}
