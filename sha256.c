#include <stdlib.h>


unsigned long rot(unsigned long num, int rotate ){
	return (num << rotate)|(num >> sizeof(num)-rotate);
}

unsigned long ** keccakfround(unsigned long  a[][5], unsigned long r){
	
	//theta step
	
	//xor parity bits
	unsigned long parity[5];
	for(int i=0;i<5;i++){
		unsigned long par = a[i][0]; 
		for(int j=1;j<5;j++){
			par ^= a[i][j];
		}
		parity[i] = par;
	}
	
	//alter a correctly
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			a[i][j] = a[i][j] ^ (parity[(i-1)%5] ^ rot(parity[(i+1)%5],1));
		}
	}

	//ro and pi and chi steps
	
	unsigned long rot_offsets[5][5] = {{ 0, 1,62,28,27},
					 {36,44, 6,55,20},
					 { 3,10,43,25,39},
					 {41,45,15,21, 8},
					 {18, 2,61,56,14}};
	//temp array
	unsigned long b[5][5];
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			b[j][(2*i+3*j)%5] = rot(a[i][j],rot_offsets[i][j]); 
		}
	}

	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			a[i][j] = b[i][j] ^ ((~b[(i+1)%5][j]) & b[(i+2)%5][j]);
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
	
}



unsigned long * keccakf(unsigned long * bl ){
	
	unsigned long ** state = malloc(25 * sizeof(unsigned long));
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			state[i][j] = *(bl+i+j);
		}
	}
	
	for(int i=0;i<24;i++){
		state = keccakfround(state, i);
	}
	
	unsigned long * res = malloc(25*sizeof(unsigned long));

	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			*(res+i+j) = state[i][j];
		}
	}

	return res;

}

