#include <stdlib.h>
#include <gmp.h>

int main(int argc, char** argv){

    mpz_t n,e_a,e_b,r,s,mcd,c_a,c_b,m,ca_powr,cb_pows;
    mpz_init(n);
    mpz_init(e_a);
    mpz_init(e_b);
    mpz_init(r);
    mpz_init(s);
    mpz_init(mcd);
    mpz_init(c_a);
    mpz_init(c_b);
    mpz_init(m);
    mpz_init(ca_powr);
    mpz_init(cb_pows);
  
    mpz_set_str(n,"734832328116465196692838283564479145339399316070744350481249855342603693301556011015131519",10);
    mpz_set_str(c_a,"93734709405860059315217893988749243105435169093069740192815939750720282941641642747093171",10);
    mpz_set_str(c_b,"269125499927676766418170761935369755935025654357890774691167668682175704132138220498309902",10);
    mpz_set_str(e_a,"17",10);
    mpz_set_str(e_b,"65537",10);

    mpz_gcdext(mcd,r,s,e_a,e_b);
    gmp_printf("mcd(e_a,e_b): %Zd | r: %Zd | s: %Zd\n",mcd,r,s);

    mpz_gcd(mcd,c_b,n);
    gmp_printf("mcd(c_b,n): %Zd \n\n", mcd);
    
    mpz_powm(ca_powr, c_a, r,n);
    mpz_powm(cb_pows, c_b, s,n);
    mpz_mul(m,ca_powr, cb_pows);
    mpz_powm_ui(m,m,1,n);

    gmp_printf("c_a: %Zd\n\nc_b: %Zd\n\n",c_a,c_b);
    gmp_printf("ca_powr: %Zd\n\ncb_pows: %Zd\n\n",ca_powr,cb_pows);
    gmp_printf("m: %Zd\n\n",m);
    return 0;
}
