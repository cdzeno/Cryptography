// Compile: gcc <filename>.c -o <fileout> -lgmp

#include <stdio.h>
#include "gmp.h"

// int mpz_probab_prime_p (const mpz_t n, int reps):
// Return 2 if n is definitely prime, 
// Return 1 if n is probably prime (without being certain) 
// Return 0 if n is definitely non-prime.

int main(int argc, char** argv){

    mpz_t n,a,b,b_2,r,pow_a,zero,one,two,factor1,factor2,is_even; 
    int is_square_root = 0;

    mpz_init(n); 
    mpz_init(a); 
    mpz_init(b); 
    mpz_init(b_2);  
    mpz_init(r);
    mpz_init(pow_a);
    
    mpz_init(factor1);
    mpz_init(factor2);
    mpz_init(is_even);

    mpz_init_set_str(zero, "0",10); 
    mpz_init_set_str(one, "1",10); 
    mpz_init_set_str(two, "2",10); 
    
    printf("Insert an odd 'n' to be factorized: ");
    gmp_scanf("%Zd", n);
    gmp_printf("You've choosen to factorize: %Zd\n",n);
    
    // Check if 'n' is odd:
    // is_odd = n^1 mod 2
    mpz_powm(is_even, n, one, two);
 
    if(mpz_cmp(is_even, zero) == 0){
        // is_even = 0 
        printf("Sorry but you've insered an even number...\n");
        return -1;
    }

    mpz_sqrtrem(a, r, n);

    if (mpz_cmp(r,zero) != 0){
        // gmp_printf("%Zd is not a perfet square... r =  %Zd\n",n,r);   
        // Prendo parte intera superiore della radice
        mpz_add(a,a,one);
    }
     
    gmp_printf("%Zd is superior square root of %Zd\n",a,n);   
    
    // ripeti finchè b_2 non è un quadrato perfetto
    do{
        // b_2 = a^2 - n
        mpz_pow_ui(pow_a, a, 2);
        mpz_sub(b_2, pow_a, n);
        // se b_2 non è un quadrato perfetto => a = a + 1
        if (mpz_perfect_square_p(b_2) ==  0){
            // a = a + 1
            mpz_add(a,a,one);
            is_square_root = 0;
        } else{
            is_square_root = 1;
        }

    }while(!is_square_root);
    
    // b = sqrt(b_2)
    mpz_sqrt(b, b_2);
    
    // factor1 = a - b
    mpz_sub(factor1, a, b);

    // factor2 = a + b
    mpz_add(factor2, a, b);

    gmp_printf("%Zd = %Zd*%Zd\n",n,factor1,factor2);   
    return 0;
}
