#include <stdlib.h>
#include <stdio.h>

#define STATE_SIZE 5
#define ROUNDS 24
#define CBLOCKS 8
#define RBLOCKS 17

unsigned long rot(unsigned long num, int rotate ){
	return (num << rotate)|(num >> (sizeof(num)-rotate));
}

unsigned long ** keccakfround(unsigned long ** a, unsigned long r){
	
	//theta step
	
	//xor parity bits
	unsigned long parity[STATE_SIZE];
	for(int i=0;i<STATE_SIZE;i++){
		unsigned long par = a[i][0]; 
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
	
	unsigned long rot_offsets[STATE_SIZE][STATE_SIZE] = {{ 0, 1,62,28,27},
					 {36,44, 6,55,20},
					 { 3,10,43,25,39},
					 {41,45,15,21, 8},
					 {18, 2,61,56,14}};
	//temp array
	unsigned long b[STATE_SIZE][STATE_SIZE];
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
	
	unsigned long round_consts[]={0x0000000000000001,
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



unsigned long * keccakf(unsigned long * bl ){

	unsigned long ** state = malloc(STATE_SIZE * sizeof(unsigned long *));
	for(int i=0;i<STATE_SIZE;i++){
		state[i] = malloc(STATE_SIZE * sizeof(unsigned long));
		for(int j=0;j<STATE_SIZE;j++){
			state[i][j] = *(bl+(i*STATE_SIZE)+j);
		}
	}

	for(int i=0;i<ROUNDS;i++){
		state = keccakfround(state, i);
	}
	
	unsigned long * res = malloc(STATE_SIZE * STATE_SIZE *sizeof(unsigned long));

	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			*(res+(i * STATE_SIZE)+j) = state[i][j];
		}
	}

	return res;

}

void add_block(unsigned long * bl, unsigned long * in, int r){
	for(int i=0;i<r;i++){
		bl[i] ^= in[i]; 
	}
}

unsigned long * sponge(unsigned long * in, int insize, int r, int c, int outsize){
	unsigned long * bl = calloc(r+c,sizeof(unsigned long));
	//absorbing
	for(int i=0;i<insize;i+=r){
		add_block(bl,in+i,r);
		bl = keccakf(bl);
	}	

	//squeezing
	unsigned long * out = malloc(outsize * r * sizeof(unsigned long));
	for(int i=0;i<outsize;i++){
		for(int j=0;j<r;j++){
			out[i*r+j] = bl[j];
		}

		bl = keccakf(bl);
	}

	free(bl);
	return out;	
}

int main(){
	unsigned long * in = malloc(17 * sizeof(unsigned long));	
	for(int i=0;i<17;i++){
		in[i] = 0x1010101010101010;
	}
	
	unsigned long * hash = sponge(in,17,RBLOCKS,CBLOCKS,17);

	for(int i=0;i<4;i++){
		printf("%lx",hash[i]);
	}
	return 0;
}
