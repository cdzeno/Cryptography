#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>

// Possibili combinazioni tra Coppie di Plaintext
#define N_PLAIN 8
#define MAX_DIM N_PLAIN * (N_PLAIN-1)

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

typedef struct _HalfKey{
  unsigned int key: 4;
} HalfKey;

L0R0 list_plaintext[8] = {{0b000111, 0b011011}, {0b101110, 0b011011}, {0b000000, 0b011011}, {0b111111, 0b011011},
                          {0b101010, 0b011011}, {0b010101, 0b011011}, {0b101101, 0b011011}, {0b110111, 0b011011}};

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
void print_plains(L0R0* lista_plain);

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

  HalfKey* key_s1 = malloc(sizeof(HalfKey)*16); // key_s1[i] = K4i = E(L4) ^ i
  HalfKey* key_s2 = malloc(sizeof(HalfKey)*16); // key_s2[i] = K4i = E(L4) ^ i

  int index1 = 0;
  int index2 = 0;

  for (uint8_t i = 0b0000; i <= 0b1111 ; i++) {
    // i = E(L4) ^ K4 = left
    // i ^ in.left = E(L4*) ^ K4 = right
    if((getBox(s1,i) ^ getBox(s1, (i ^ in.left))) == out.left){
      key_s1[index1].key = i ^ (expansion(c1.l0) >> 4);
      //printf("key_s1[%d]: 0x%x\n", index1, key_s1[index1].key);
      index1++;
    }
  }

  // Bruteforce S2: 0000 => 1111
  for (uint8_t i = 0b0000; i <= 0b1111 ; i++) {
    if((getBox(s2,i) ^ getBox(s2, (i ^ in.right))) == out.right){
      key_s2[index2].key = i ^ (expansion(c1.l0) & 0b00001111);
      //printf("key_s2[%d]: 0x%x\n", index2, key_s2[index2].key);
      index2++;
    }
  }

  uint8_t* ListGenKey = malloc(sizeof(uint8_t)*((index1*index2)));
  int index3 = 0;

  for (int i = 0; i < index1; i++) {
    for (int j = 0; j < index2; j++) {
      ListGenKey[index3] =  (((uint8_t)key_s1[i].key) << 4) | key_s2[j].key;
      //printf("ListGenKey[%d]: 0x%x\n", index3, ListGenKey[index3]);
      index3++;
    }
  }

  length_gen_key[length_index] = index3;
  return ListGenKey;
}

uint8_t** gen_key (L0R0* lista){
  // length Lista = 8
  // 56 = 8 * 7 => MAX Coppie Plaintext
  uint8_t** list_key = malloc(sizeof(uint8_t*)*MAX_DIM);

  for (int i = 0; i < N_PLAIN; i++) {
    for (int j = 0; j < N_PLAIN-1; j++) {
      if(i != j){
          list_key[length_index] = diff_crypto(lista[i], lista[j], des(lista[i], key, 3), des(lista[j], key, 3));
          length_index++;
      }
    }
  }

  return list_key;
}

void get_key(uint8_t** list_key){
  // 256 = 2^8 = Tutte le possibili chiavi
  int* all_key = malloc(sizeof(uint8_t)* 256);
  int n_keys = 0;

  printf("\n------ KEY COMPUTATION ------\n");

  for (int i = 0; i < length_index; i++) {
    //printf("length_gen_key[%d]: %d\n", i, length_gen_key[i]);
    for (int j = 0; j < length_gen_key[i]; j++) {
        all_key[list_key[i][j]]++;
        n_keys++;
    }
  }

  printf("\n%d different list of keys... \n", length_index);
  printf("%d total 8-bit keys...\n", n_keys);

  for (int i = 0; i < 256; i++) {
    if(all_key[i] == length_index)
      printf("K4 is: 0x%x\n", i);
  }

}

void print_gen_key (uint8_t** gen_key){

  printf("\n -------- LIST KEY -------- \n");

  for (int i = 0; i < length_index; i++) {
    printf("[%d]: ", i);
    for (int j = 0; j < length_gen_key[i]; j++) {
      printf("(0x%x) ", gen_key[i][j]);
    }
    printf("\n");
  }
}

void print_plains(L0R0* lista_plain){

  printf("\n+ --------- LIST PLAINTEXTs ---------- + --------- LIST CIPHERTEXTs ---------- +\n");

  for (int i = 0; i < N_PLAIN; i++) {
    printf("| Plaintext[%d]: left=0x%.2X | right=0x%.2X | Ciphertext[%d]: left=0x%.2X | right=0x%.2X |\n", i, lista_plain[i].l0, lista_plain[i].r0, i, des(lista_plain[i], key, 3).l0, des(lista_plain[i], key, 3).r0);
  }
  printf("+ ------------------------------------ + ------------------------------------- +\n");
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
  (void)strncpy(key_string, argv[1], 9);
  key.key = bin_string_to_int(key_string);

  printf("\n\t\t======= DIFFERENTIAL CRYPTANALYSIS DES 3 round =======\n");
  print_plains(list_plaintext);
  uint8_t** chiavi = gen_key(list_plaintext);


  get_key(chiavi);

  return 0;
}
