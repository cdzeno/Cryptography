#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "string.h"

// Possibili combinazioni tra Coppie di Plaintext
#define MAX_DIM 2

typedef struct _L0R0
{
  unsigned int l0: 6;
  unsigned int r0: 6;
} L0R0;

typedef struct _Key
{
  unsigned int key: 9;
} Key;

typedef struct _InSBox{
  unsigned int left:4;
  unsigned int right:4;
} InSBox;

typedef struct _OutSBox{
  unsigned int left:3;
  unsigned int right:3;
} OutSBox;

typedef struct _CoupleKey{
  unsigned int left:4;
  unsigned int right:4;
} CoupleKey;

L0R0 list_plaintext[8] = {{0b000111, 0b011011}, {0b101110, 0b011011}, {0b000000, 0b011011}, {0b111111, 0b011011},
                          {0b101010, 0b011011}, {0b010101, 0b011011}, {0b101101, 0b011011}, {0b110111, 0b011011}};

L0R0 list_plaintext2[2] = {{0b000111, 0b011011}, {0b101110, 0b011011}};
uint8_t s1 [2][8] = {{5,2,1,6,3,4,7,0},{1,4,6,2,0,7,5,3}};
uint8_t s2 [2][8] = {{4,0,6,5,7,1,3,2},{5,3,0,7,6,2,1,4}};
int length_gen_key[MAX_DIM];   // array lunghezze elementi di uint8_t** gen_key
int length_index = 0;  // ultima posizione "righe" di uint8_t** gen_key
Key key;

uint8_t expansion (uint8_t r0); //in: 6bit, out: 8bit
uint8_t getBox (uint8_t sbox[2][8], uint8_t in_sbox); //in: mat3bit 4bit, out: 3bit
uint8_t feistel (uint8_t r0, uint8_t key); //in: 6bit 8bit, out: 6bit
L0R0 turn (L0R0 input, uint8_t key); //in: L0R0 8bit, out: L0R0
uint8_t keySchedule (Key key, uint8_t n_round);//in: 9bit 4bit, out: 8bit
unsigned int bin_string_to_int (char* s); //in: string binary digit out: unsigned int
L0R0 des (L0R0 plaintext, Key key, uint64_t n_round);
uint8_t* diff_crypto (L0R0 p1, L0R0 p2, L0R0 c1, L0R0 c2);
uint8_t** gen_key (L0R0* lista_plain);
void get_key(uint8_t** list_key);

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

L0R0 des (L0R0 plaintext, Key key, uint64_t n_round)
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
  //printf("Ciphertext: left=0x%x  right=0x%x\n", input.l0, input.r0);
  return input;
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

uint8_t* diff_crypto (L0R0 p1, L0R0 p2, L0R0 c1, L0R0 c2){
  InSBox in = {.left = (expansion(c1.l0) ^ expansion(c2.l0)) >> 4,
               .right = (expansion(c1.l0) ^ expansion(c2.l0)) & 0b00001111};

  OutSBox out = {.left = ((c1.r0 ^ c2.r0) ^ (p1.l0 ^ p2.l0)) >> 3,
                 .right =   ((c1.r0 ^ c2.r0) ^ (p1.l0 ^ p2.l0)) & 0b000111};

  //printf("InSbox: left=0x%x | right=0x%x\n", in.left, in.right);
  //printf("OutSbox: left=0x%x | right=0x%x\n", out.left, out.right);
  // Bruteforce S1: 0000 => 1111
  CoupleKey* key_list1 = malloc(sizeof(CoupleKey)*16);
  CoupleKey* key_list2 = malloc(sizeof(CoupleKey)*16);

  int index1 = 0;
  int index2 = 0;

  for (uint8_t i = 0b0000; i <= 0b1111 ; i++) {
    if((getBox(s1,i) ^ getBox(s1, (i ^ in.left))) == out.left){
      key_list1[index1].left = i;
      key_list1[index1].right = i ^ in.left;
      index1++;
    }
  }
  // PROBLEMA = E(L4) ^ K4 = i
  // Bruteforce S2: 0000 => 1111
  for (uint8_t i = 0b0000; i <= 0b1111 ; i++) {
    if((getBox(s2,i) ^ getBox(s2, (i ^ in.right))) == out.right){
      //printf("i ^ in.right: 0x%x | i: 0x%x | in.right: 0x%x | getBox(s2,%d): 0x%x | getBox(s2, (%d ^ in.right): 0x%x\n", i ^ in.right, i, in.right, i, getBox(s2,i), i, getBox(s2, (i ^ in.right)) );;
      printf("expansion: 0x%x\n", (0b00001111 & expansion(c1.r0)));
      key_list2[index2].left =  i ^  (0b00001111 & expansion(c1.r0));
      key_list2[index2].right = i ^ in.right;
      index2++;
    }
  }

  uint8_t* LastKey = malloc(sizeof(uint8_t)*((index1*index2) + 1));
  int index3 = 0;

  for (int i = 0; i < index1; i++) {
    for (int j = 0; j < index2; j++) {
      LastKey[index3] = (((uint8_t)(key_list1[i].left)) << 4) | key_list2[j].right;
      //LastKey[index3] = (((uint8_t)(key_list1[i].left)) << 4) | (0b00001111 & key_list2[j].right);
      printf("LastKey[%d]: 0x%x |left =x%x key_list2[%d]: 0x%x\n", index3, LastKey[index3],(((uint8_t)(key_list1[i].left)) << 4) , j, key_list2[j].right);
      index3++;
    }
  }

  length_gen_key[length_index] = index3;
  //LastKey[index3] = '0x0'
  return LastKey;
}

uint8_t** gen_key (L0R0* lista){
  // length Lista = 8
  // 56 = 8 * 7 => MAX Coppie Plaintext
  uint8_t** list_key = malloc(sizeof(uint8_t*)*MAX_DIM);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 1; j++) {
      if(i != j){
          list_key[length_index] = diff_crypto(lista[i], lista[j], des(lista[i], key, 3), des(lista[j], key, 3));
          length_index++;
      }
    }
  }

  return list_key;
}

void print_gen_key (uint8_t** gen_key){
  //uint8_t** r = gen_key;
  //uint8_t* c = gen_key[0];

  printf("\n -------- length_index %d \n", length_index);
  for (int i = 0; i < length_index; i++) {
    printf("[%d]: ", i);
    for (int j = 0; j < length_gen_key[i]; j++) {
      printf("(0x%x) ", gen_key[i][j]);
    }
    printf("\n");
  }

}

int main(int argc, char* argv[])
{
  // argv[1] = 12 bit plaintext
  // argv[2] = 9 bit key
  assert(argc == 2);
  assert((int)strlen(argv[1]) == 9);
  //assert((int)strlen(argv[1]) == 12 && (int)strlen(argv[2]) == 9);

  char l0[7] = {'0','0','0','0','0','0','\0'};
  char r0[7] = {'0','0','0','0','0','0','\0'};
  char key_string[10] = {'0','0','0','0','0','0','0','0','0','\0'};
  //(void)strncpy(l0, argv[1], 6);
  //(void)strncpy(r0, argv[1]+6, 6);
  (void)strncpy(key_string, argv[1], 9);

  printf("Key: 0b%s\n", key_string);

  //L0R0 plain1 = {.l0 = bin_string_to_int(l0), .r0 = bin_string_to_int(r0)};
  //L0R0 plain2 = {.l0 = 0b000111, .r0 = 0b011011};
  key.key = bin_string_to_int(key_string);
  //L0R0 cipher1 = des(plain1, key_plain, 3);
  //L0R0 cipher2 = des(plain2, key_plain, 3);
  //diff_crypto(plain1, plain2, cipher1, cipher2);
  uint8_t* ris = diff_crypto(list_plaintext2[0], list_plaintext2[1], des(list_plaintext2[0], key, 3), des(list_plaintext2[1], key, 3));
  //uint8_t** chiavi = gen_key(list_plaintext2);
  //print_gen_key(chiavi);

  for (int i = 0; i < 4; i++) {
    printf("ris[%d]: 0x%x\n", i,ris[i]);
  }

  return 0;
}
