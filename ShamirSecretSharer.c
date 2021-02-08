#include <string.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#define POLY_CONST_SIZE 256
#define SHARE_X_SIZE 256
#define MAESENNE_PRIME_EXPONENT 521
#define MAX_SHARE_LINE_SIZE 270
#define IO_BASE 16


void init_mpz_array(mpz_t vec[], int n){
	for(int i=0;i<n;i++){
		mpz_init(vec[i]);
	}
}

void clear_mpz_array(mpz_t vec[], int n){
	for(int i=0;i<n;i++){
		mpz_clear(vec[i]);	
	}
}

//calculates the 'Li(0)' value for the shares. vecx is the x's of the shares, n is the amount of shares. 
void lagrange_basis(mpz_t res, int n, mpz_t vecx[], int i, mpz_t mod){
	//initializes res
	mpz_set_ui(res,1);

	//calculates (-xk)/(xi-xk) for each k!=i
	for(int k=0;k<n;k++){
		if(k==i)
			continue;
		mpz_t tmp1, tmp2;
		mpz_inits(tmp1,tmp2,NULL);
		mpz_sub(tmp1,mod,vecx[k]);//-xk in the integer field of mod
		mpz_sub(tmp2,vecx[i],vecx[k]);//calculates xi-xk
		if(mpz_sgn(tmp2) < 0)//if xi-xk is negative, calculates the additive inverse
			mpz_add(tmp2,mod,tmp2);	
		mpz_invert(tmp2,tmp2,mod);//calculates the multiplicative inverse
		mpz_mul(tmp1,tmp1,tmp2);
		mpz_mul(res,res,tmp1);//multiplies res by (-xk)/(xi-xk)
		mpz_mod(res,res,mod);
		mpz_clears(tmp1,tmp2,NULL);
	}
}

//a function for interpolating the secret from shares in vecx and vecy,in polynomial of degree polysize-1
void interpolate(mpz_t res,mpz_t vecx[],mpz_t vecy[], int polysize, mpz_t mod){
	//clears res
	mpz_set_ui(res,0);
	//calculates sum of yi*Li(0)
	for(int i=0;i<polysize;i++){
		mpz_t lag;	
		mpz_init(lag);
		lagrange_basis(lag,polysize,vecx,i,mod);//claculates Li(0)
		mpz_mul(lag,lag,vecy[i]);
		mpz_add(res,res,lag);
		mpz_clear(lag);
	}
	mpz_mod(res,res,mod);//claculates res in mod

	return;
}

//calculates the y for an x and a polyvec array where polyvec[i] is the constant of the x of the i+1 degree, and the free constant is the secret. 
void  calc_y(mpz_t res, mpz_t polyvec[],int polydeg,mpz_t x, mpz_t secret,mpz_t mod){
	
	mpz_set_ui(res,0);

	for(int i=0;i<polydeg;i++){
		mpz_t tmp;
		mpz_init(tmp);
		mpz_powm_ui(tmp,x,i+1,mod);//power and modulu
		mpz_mul(tmp,tmp,polyvec[i]);
		mpz_add(res,res,tmp);
		mpz_mod(res,res,mod);
		mpz_clear(tmp);
	}
	
	mpz_add(res,res,secret);
	mpz_mod(res,res,mod);//mods after adding the secret
	return;
}

//generates the needed shares into vecx and vecy, where min_shares is the minimum number of shares needed to uncover the secret and sharew amont is the number of shares made.
void generate_shares(mpz_t vecx[],mpz_t vecy[],int min_shares, int share_amount,mpz_t secret, mpz_t mod){
	//sets up random number generator
	gmp_randstate_t rand;
	gmp_randinit_default(rand);
	
	//sets up polyvec to have constants of 256 bit integers
	mpz_t polyvec[min_shares-1];
	init_mpz_array(polyvec,min_shares-1);
	for(int i=0;i<min_shares-1;i++){
		mpz_urandomb(polyvec[i],rand,POLY_CONST_SIZE);
	}


	//creates random shares, where x is a random 256 bit integer
	for(int i=0;i<share_amount;i++){
		mpz_urandomb(vecx[i],rand,SHARE_X_SIZE);
		calc_y(vecy[i],polyvec,min_shares-1,vecx[i],secret,mod);
	}	
	for(int i=0;i<min_shares-1;i++)	
		mpz_clear(polyvec[i]);	

	return;
}
//gets number of shares in fp, which is the number of lines
int get_share_num(FILE * fp){
	int n=0;
	char buf[MAX_SHARE_LINE_SIZE];
	for(n=0;!feof(fp);n++){
		fgets(buf,MAX_SHARE_LINE_SIZE,fp);
	}
	rewind(fp);
	n--;
	return n;
}

//gets shares from file fp into vecx and vecy
void getshares(mpz_t vecx[],mpz_t vecy[], FILE * fp, int n){
	char buf[MAX_SHARE_LINE_SIZE];//stores the share as text
	for(int i=0;i<n;i++){
		fgets(buf,MAX_SHARE_LINE_SIZE,fp);
		char * xstr = strtok(buf+1,",");//takes the x without the "(" in the start
		char * ystr = strtok(NULL,",");//gets the y
		//removes ')' in the end of y 
		ystr = strtok(ystr,")");
		//puts num from string to the vecs
		mpz_set_str(vecx[i],xstr,IO_BASE);
		mpz_set_str(vecy[i],ystr,IO_BASE);
	}
}

int main(int argc, char *argv[]){
	//initiaizes "mod" to be the 13th mersenne prime
	mpz_t mod;
	mpz_init(mod);
	mpz_ui_pow_ui(mod,2,MAESENNE_PRIME_EXPONENT);
	mpz_sub_ui(mod,mod,1);

	if(argc<3){
		return 0;
	}
	if(!strcmp(argv[1],"g") && argc > 4){ //if in "generate" mode

		int min_group_size = atoi(argv[2]);
		int share_num = atoi(argv[3]);
		mpz_t secret;
		mpz_init_set_str(secret,argv[4],16);
		mpz_t vecx[share_num], vecy[share_num];
		init_mpz_array(vecx,share_num);//inits arrays
		init_mpz_array(vecy,share_num);
		generate_shares(vecx,vecy,min_group_size,share_num,secret,mod);//generates the shares
		for(int i=0;i<share_num;i++){//prints shares in hex
			gmp_printf("(%Zx,%Zx)\n",vecx[i],vecy[i]);
		}
		
		//clears vars
		clear_mpz_array(vecx,share_num);
		clear_mpz_array(vecy,share_num);	
		mpz_clear(secret);	


	}if(!strcmp(argv[1],"d") && argc > 2){ //if in "decrypt" mode
		
		FILE * fp = fopen(argv[2],"r");
		//saves amount of shares
		int share_num = get_share_num(fp);
		mpz_t vecx[share_num] , vecy[share_num];		
		
		//inits share vecs
		init_mpz_array(vecx,share_num);
		init_mpz_array(vecy,share_num);	
		
		//gets shares into share vecs
		getshares(vecx,vecy,fp,share_num);
		mpz_t secret;

		//interpolaes secret
		mpz_init(secret);
		interpolate(secret,vecx,vecy,share_num,mod);
		gmp_printf("%ZX",secret);

		//clears vars
		clear_mpz_array(vecx,share_num);
		clear_mpz_array(vecy,share_num);
		mpz_clear(secret);
	}
	//clears mod
	mpz_clear(mod);

	return 0;
}
