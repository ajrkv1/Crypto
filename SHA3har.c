#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STATE_SIZE 5
#define ROUNDS 24
#define CBLOCKS 8
#define RBLOCKS 17

typedef unsigned long block;

block rot(block num, int rotate ){
	return (num << rotate)|(num >> (sizeof(num)-rotate));
}

block ** keccakfround(block ** a, block r){
	
	//theta step
	
	//xor parity bits
	block parity[STATE_SIZE];
	for(int i=0;i<STATE_SIZE;i++){
		block par = a[i][0]; 
		for(int j=1;j<STATE_SIZE;j++){
			par ^= a[i][j];
		}
		parity[i] = par;
	}
	
	//alter a correctly
	for(int i=0;i<STATE_SIZE;i++){
		for(int j=0;j<STATE_SIZE;j++){
			a[i][j] = a[i][j] ^ (parity[(i-1)%STATE_SIZE] ^ rot(parity[(i+1)%STATE_SIZE],1));
		}
	}

	//ro and pi and chi steps
	
	block rot_offsets[STATE_SIZE][STATE_SIZE] = {{ 0, 1,62,28,27},
					 {36,44, 6,55,20},
					 { 3,10,43,25,39},
					 {41,45,15,21, 8},
					 {18, 2,61,56,14}};
	//temp array
	block b[STATE_SIZE][STATE_SIZE];
	for(int i=0;i<STATE_SIZE;i++){
		for(int j=0;j<STATE_SIZE;j++){
			b[j][(2*i+3*j)%STATE_SIZE] = rot(a[i][j],rot_offsets[i][j]); 
		}
	
	}
	for(int i=0;i<STATE_SIZE;i++){
		for(int j=0;j<STATE_SIZE;j++){
			a[i][j] = b[i][j] ^ ((~b[(i+1)%STATE_SIZE][j]) & b[(i+2)%STATE_SIZE][j]);
		}
	}

	//iota step
	
	block round_consts[]={0x0000000000000001,
				      0x0000000000008082,
				      0x800000000000808A,
				      0x8000000080008000,
			 	      0x000000000000808B,
				      0x0000000080000001,
	  			      0x8000000080008081,
				      0x8000000000008009,
				      0x000000000000008A,
				      0x0000000000000088,
				      0x0000000080008009,
				      0x000000008000000A,
				      0x000000008000808B,
				      0x800000000000008B,
				      0x8000000000008089,
				      0x8000000000008003,
				      0x8000000000008002,
				      0x8000000000000080,
				      0x000000000000800A,
				      0x800000008000000A,
				      0x8000000080008081,
				      0x8000000000008080,
				      0x0000000080000001,
				      0x8000000080008008};

	a[0][0] = a[0][0] ^ round_consts[r];
	return a;	
}




block * keccakf(block * bl ){

	block ** state = malloc(STATE_SIZE * sizeof(block *));
	for(int i=0;i<STATE_SIZE;i++){
		state[i] = malloc(STATE_SIZE * sizeof(block));
		for(int j=0;j<STATE_SIZE;j++){
			state[i][j] = *(bl+(i*STATE_SIZE)+j);
		}
	}

	for(int i=0;i<ROUNDS;i++){
		state = keccakfround(state, i);
	}
	
	block * res = malloc(STATE_SIZE * STATE_SIZE *sizeof(block));

	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			*(res+(i * STATE_SIZE)+j) = state[i][j];
		}
	}
	free(state);
	return res;

}

void add_block(block * bl, block * in, int r){
	for(int i=0;i<r;i++){
		bl[i] ^= in[i]; 
	}
}

block * sponge(void * in, int insize, int r, int c, int outsize){
	block * bl = calloc(r+c,sizeof(block));
	
	//padding
	int padsize = r * sizeof(block) - insize%(r * sizeof(block));
	block * padded = calloc(insize+padsize,1);
	for(int i=0;i<insize;i++){
		((char *)padded)[i] = ((char *)in)[i];	
	}	
	((char *)padded)[insize] = (char)0x06;

	
	((char *)padded)[insize+padsize-1] =(char)0x80;
	insize+=padsize;

	//absorbing
	for(int i=0;i<insize/sizeof(block);i+=r){
		add_block(bl,padded+i,r);
		bl = keccakf(bl);
	}	

	//squeezing
	block * out = malloc(outsize * r * sizeof(block));
	for(int i=0;i<outsize;i+=r){
		for(int j=0;j<r;j++){
			out[i*r+j] = bl[j];
		}

		bl = keccakf(bl);
	}

	free(bl);
	free(padded);
	return out;
}

block * sha256(void * in, int insize){
	return sponge(in,insize,RBLOCKS,CBLOCKS,4);
}

int main(int argc, char *argv[]){
		
	block * hash = sha256(argv[1],strlen(argv[1]));

	for(int i=0;i<4;i++){
		printf("%lX",hash[i]);
	}
	return 0;
}
