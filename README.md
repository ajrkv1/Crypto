# Crypto

In here, there are some cryptographic functions, algorithms, and schemes I implemented.
##### IMPORTANT!
Never, but **NEVER**, use these implementations in your projects. I'm not a security expert, and these implementations probably are totally insecure. They are probably so insecure they'll be bullied at highschool for implementations if that was a thing. Dont use them. You dont want to have a security issue in your project because one of my stupid mistakes. That said - if you find a vulnerability in one of these implementations, please notify me about it!

### What's in here?
Here's a list of what is in here right now:
  - SHA3har hash algorithm
  - Shamir's Secret Sharing Scheme
  
 More information on those ahead!
 
 ### SHA3har 
 Once apon a time, there was a person named Shahar. Shahar wanted to implement SHA3 on his own, so he tried. He studied and worked on the implementaion, until everything was ready! But, when he tried it, he found out something horrable! it does not give the same hash as the SHA3 algorithm! after concluding (or to be more precise, hoping) that the issue is in the padding scheme, he decided to rename the hashing function from "SHA3" to "SHA3har", which stands for "SHA3? hmm... ain't really", and use it. 
 
 The 'SHA3har' algorithm is a shot at implementing the Keccak algorithm, which is more broadly named "SHA3". It is built as a sponge construction function, which can be used as a hash function, a stream cipher, and more.
 
 **note:** As I'm writing this, sha3har is not finished. The algorithm itself is finished but not the command line program that activates it. 
 #### Sources of information I used in this implementation:
 * The Wikipedia on the SHA-3 hashing algorithm - https://en.wikipedia.org/wiki/SHA-3
 * The "Team Keccak" website - https://keccak.team
 
 ### Shamir Secret Sharing Scheme
 The "Shamir Secret Sharing Scheme" is a way to distribute a secret between a group of users, such that any sub group above a specified thresh will be able to uncover the secret, but any group of people below the thresh wont be able to uncover it. 
 This is done for a group of n people, and a thresh of r people, by creating a random polynomial of the r-1th degree with the secret as the free constant, and distributing to each person a "share" which is composed of a point in the polynomial. Because every r points specify a specific r-1 degree polynmial, we can interpolate the secret from any r shares.
 
 In my implementation, I use the "Lagrange Interpolation" to interpolate the secret from r shares, in a finite field of mod 2^251-1, the 13th Mersenne number. For computations, I use the GMP library. 
 
 #### How to use it?
after compiling the implementation with the gmp library, use the arguments like this: ```g [Thresh] [Share amount] [Secret]```, where ```Thresh``` is the minimal number of shares needed to uncover the secret, ```Share amount``` is the minimal amount of shares you'd want to create, and ```Secret``` is the secret you want to share, in Hexadecimal. **Note:** ```Secret``` must be shorter than 520 bits. 
 
 #### Sources of information I used in this implementation:
* The Wikipedia article on "Shamir Secret Sharing" - https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing
* The Wikipedia article on "Lagrange Polynomials" - https://en.wikipedia.org/wiki/Lagrange_polynomial
* The GMP library ant it's Documentation - https://gmplib.org/
