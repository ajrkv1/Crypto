#include <string.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#define POLY_CONST_SIZE 256
#define SHARE_X_SIZE 256
#define MAESENNE_PRIME_EXPONENT 521
#define MAX_SHARE_LINE_SIZE 270
void lagrange_basis(mpz_t res, int n, mpz_t vecx[], int i, mpz_t mod){
	mpz_set_ui(res,1);
	for(int k=0;k<n;k++){
		if(k==i)
			continue;
		mpz_t tmp1, tmp2;
		mpz_inits(tmp1,tmp2,NULL);
		mpz_sub(tmp1,mod,vecx[k]);	
		mpz_sub(tmp2,vecx[i],vecx[k]);
		if(mpz_sgn(tmp2) < 0)
			mpz_add(tmp2,mod,tmp2);	
		mpz_invert(tmp2,tmp2,mod);
		mpz_mul(tmp1,tmp1,tmp2);
		mpz_mul(res,res,tmp1);
		mpz_mod(res,res,mod);
		mpz_clears(tmp1,tmp2,NULL);
	}
}


void interpolate(mpz_t res,mpz_t vecx[],mpz_t vecy[], int polysize, mpz_t mod){
	
	mpz_set_ui(res,0);

	for(int i=0;i<polysize;i++){
		mpz_t lag;
		mpz_init(lag);
		lagrange_basis(lag,polysize,vecx,i,mod);	
		mpz_mul(lag,lag,vecy[i]);
		mpz_add(res,res,lag);
		mpz_clear(lag);
	}
	mpz_mod(res,res,mod);

	return;
}

void  calc_y(mpz_t res, mpz_t polyvec[],int polydeg,mpz_t x, mpz_t secret,mpz_t mod){
	
	mpz_set_ui(res,0);

	for(int i=0;i<polydeg;i++){
		mpz_t tmp;
		mpz_init(tmp);
		mpz_powm_ui(tmp,x,i+1,mod);
		mpz_mul(tmp,tmp,polyvec[i]);
		mpz_add(res,res,tmp);
		mpz_mod(res,res,mod);
		mpz_clear(tmp);
	}
	
	mpz_add(res,res,secret);
	mpz_mod(res,res,mod);
	return;
}

void generate_shares(mpz_t vecx[],mpz_t vecy[],int min_shares, int share_amount,mpz_t secret, mpz_t mod){
	
	gmp_randstate_t rand;
	gmp_randinit_default(rand);

	mpz_t polyvec[min_shares-1];
	for(int i=0;i<min_shares-1;i++){
		mpz_init(polyvec[i]);
		mpz_urandomb(polyvec[i],rand,POLY_CONST_SIZE);
	}

	for(int i=0;i<share_amount;i++){
		mpz_urandomb(vecx[i],rand,SHARE_X_SIZE);
		calc_y(vecy[i],polyvec,min_shares-1,vecx[i],secret,mod);
	}	
	for(int i=0;i<min_shares-1;i++)	
		mpz_clear(polyvec[i]);	

	return;
}

int getpoints(mpz_t vecx[],mpz_t vecy[],int n, FILE * fp){
	char buf[MAX_SHARE_LINE_SIZE];
	for(int i=0;!feof(fp)&&i<n;i++){
		fgets(buf,MAX_SHARE_LINE_SIZE,fp);
		char * xstr = strtok(buf+1,",");
		char * ystr = strtok(NULL,",");
		ystr[strlen(ystr)-2] = '\0';
		mpz_set_str(vecx[i],xstr,16);
		mpz_set_str(vecy[i],ystr,16);
	}
	
	return 0;
}

int main(int argc, char *argv[]){

	mpz_t mod;
	mpz_init(mod);
	mpz_ui_pow_ui(mod,2,521);
	mpz_sub_ui(mod,mod,1);

	if(argc<3){
		return 0;
	}
	if(!strcmp(argv[1],"g") && argc > 4){

		int min_group_size = atoi(argv[2]);
		int share_number = atoi(argv[3]);
		mpz_t secret;
		mpz_init_set_str(secret,argv[4],16);
		mpz_t vecx[share_number], vecy[share_number];
		for(int i=0;i<share_number;i++)
			mpz_inits(vecx[i],vecy[i],NULL);
		generate_shares(vecx,vecy,min_group_size,share_number,secret,mod);
		for(int i=0;i<share_number;i++){
			gmp_printf("(%Zx,%Zx)\n",vecx[i],vecy[i]);
		}
		return 0;
		

	}if(!strcmp(argv[1],"v") && argc > 3){
		
		FILE * fp = fopen(argv[2],"r");
		int share_num = atoi(argv[3]);
		mpz_t vecx[share_num] , vecy[share_num];		
		for(int i=0;i<share_num;i++){
			mpz_inits(vecx[i],vecy[i],NULL);
		}
			
		getpoints(vecx,vecy,share_num,fp);
		mpz_t secret;
		mpz_init(secret);
		interpolate(secret,vecx,vecy,share_num,mod);
		gmp_printf("%ZX",secret);
	}

	


	return 0;
}
