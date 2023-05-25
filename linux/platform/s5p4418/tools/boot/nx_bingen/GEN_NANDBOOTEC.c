

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "NSIH.h"
#include "GEN_NANDBOOTEC.h"



unsigned int alpha_to[nn_max], index_of[nn_max];	// Galois field
int gg[rr_max];										// Generator polynomial
int T_G[rr_max][rr_max], T_G_R[rr_max][rr_max];		// Parallel lookahead table
int T_G_R_Temp[rr_max][rr_max];
int gen_roots[32769], gen_roots_true[32769];		// Roots of generator polynomial
int Parallel = 8;

unsigned int iSectorSize;
unsigned int iNX_BCH_VAR_K;							// (512 * 8) or (1024 * 8)
unsigned int iNX_BCH_VAR_M;							// 13 or 14
unsigned int iNX_BCH_VAR_T;							// 24 or 60
unsigned int iNX_BCH_VAR_N;
unsigned int iNX_BCH_VAR_R;

unsigned int iNX_BCH_VAR_TMAX;
unsigned int iNX_BCH_VAR_RMAX;

unsigned int iNX_BCH_VAR_R32;
unsigned int iNX_BCH_VAR_RMAX32;

unsigned int iNX_BCH_OFFSET;

//------------------------------------------------------------------------------
// matin
void generate_gf( void )
/* Generate GF(2**mm) from the primitive polynomial p(X) in p[0]..p[mm]
   The lookup table looks like:
   index -> polynomial form:   alpha_to[ ] contains j = alpha**i;
   polynomial form -> index form:  index_of[j = alpha**i] = i
   alpha_to[1] = 2 is the primitive element of GF(2**mm)
 */
{
	unsigned int i, mask, p;
	// Primitive polynomials
	// mm = 2^?
	if (iNX_BCH_VAR_M == 13)	p = 0x25AF;
	if (iNX_BCH_VAR_M == 14)	p = 0x41D5;

	// Galois field implementation with shift registers
	// Ref: L&C, Chapter 6.7, pp. 217
	mask = 1 ;
	alpha_to[iNX_BCH_VAR_M] = 0 ;

	for (i = 0; i < iNX_BCH_VAR_M; i++)
	{
		alpha_to[i] = mask ;
		index_of[alpha_to[i]] = i ;

		if (((p>>i)&0x1) != 0)	alpha_to[iNX_BCH_VAR_M] ^= mask ;

		mask <<= 1 ;
	}

	index_of[alpha_to[iNX_BCH_VAR_M]] = iNX_BCH_VAR_M ;
	mask >>= 1 ;

	for (i = iNX_BCH_VAR_M + 1; i < (iNX_BCH_VAR_N); i++)
	{
		if (alpha_to[i-1] >= mask)
			alpha_to[i] = alpha_to[iNX_BCH_VAR_M] ^ ((alpha_to[i-1] ^ mask) << 1) ;
		else alpha_to[i] = alpha_to[i-1] << 1 ;

		index_of[alpha_to[i]] = i ;
	}
	index_of[0] = -1 ;
}

void gen_poly( void )
/* Compute generator polynomial of the tt-error correcting Binary BCH code
 * g(x) = LCM{M_1(x), M_2(x), ..., M_2t(x)},
 * where M_i(x) is the minimal polynomial of alpha^i by cyclotomic cosets
 */
{
	int i, j, iii, jjj, Temp ;
	int rr, kk;

	// Initialization of gen_roots
	for (i = 0; i <= (iNX_BCH_VAR_N); i++)
	{
		gen_roots_true[i] = 0;
		gen_roots[i] = 0;
	}

	// Cyclotomic cosets of gen_roots
	for (i = 1; i <= 2*iNX_BCH_VAR_T ; i++)
	{
		for (j = 0; j < iNX_BCH_VAR_M; j++)
		{
			Temp = ((1<<j) * i) % (iNX_BCH_VAR_N);
			gen_roots_true[Temp] = 1;
		}
	}

	rr = 0;		// Count the number of parity check bits
	for (i = 0; i < (iNX_BCH_VAR_N); i++)
	{
		if (gen_roots_true[i] == 1)
		{
			rr++;
			gen_roots[rr] = i;
		}
	}
	kk = (iNX_BCH_VAR_N) - rr;

	// Compute generator polynomial based on its roots
	gg[0] = 2 ;	// g(x) = (X + alpha) initially
	gg[1] = 1 ;
	for (i = 2; i <= rr; i++)
	{
		gg[i] = 1 ;
		for (j = i - 1; j > 0; j--)
		{
			if (gg[j] != 0)
				gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]] + index_of[alpha_to[gen_roots[i]]]) % (iNX_BCH_VAR_N)] ;
			else
				gg[j] = gg[j-1] ;
		}
		gg[0] = alpha_to[(index_of[gg[0]] + index_of[alpha_to[gen_roots[i]]]) % (iNX_BCH_VAR_N)] ;
	}

	// for parallel encoding and syndrome computation
	// Max parallalism is rr
	if (Parallel > rr)
		Parallel = rr ;

	// Construct parallel lookahead matrix T_g, and T_g**r from gg(x)
	// Ref: Parallel CRC, Shieh, 2001
	for (i = 0; i < rr; i++)
		for (j = 0; j < rr; j++)
			T_G[i][j] = 0;

	for (i = 1; i < rr; i++)
		T_G[i][i-1] = 1 ;

	for (i = 0; i < rr; i++)
		T_G[i][rr - 1] = gg[i] ;

	for (i = 0; i < rr; i++)
		for (j = 0; j < rr; j++)
			T_G_R[i][j] = T_G[i][j];

	// Compute T_G**R Matrix
	for (iii = 1; iii < Parallel; iii++)
	{
		for (i = 0; i < rr; i++)
			for (j = 0; j < rr; j++)
			{
				Temp = 0;
				for (jjj = 0; jjj < rr; jjj++)
					Temp = Temp ^ T_G_R[i][jjj] * T_G[jjj][j];

				T_G_R_Temp[i][j] = Temp;
			}

		for (i = 0; i < rr; i++)
			for (j = 0; j < rr; j++)
				T_G_R[i][j] = T_G_R_Temp[i][j];
	}
}

void parallel_encode_bch( unsigned int *pData32, unsigned int *pECC )
/* Parallel computation of n - k parity check bits.
 * Use lookahead matrix T_G_R.
 * The incoming streams are fed into registers from the right hand
 */
{
	unsigned int i, j, iii, Temp, bb_temp[rr_max] ;
	unsigned int loop_count ;
	unsigned int bb[rr_max/32];

	// Determine the number of loops required for parallelism.
	loop_count = iSectorSize;

	// Initialize the parity bits.
	for (i = 0; i < rr_max/32; i++)
		bb[i] = 0;

	// Compute parity checks
	// S(t) = T_G_R [ S(t-1) + M(t) ]
	// Ref: Parallel CRC, Shieh, 2001

	for( iii = 0; iii<loop_count; iii++ )
	{
		for (i = 0; i < iNX_BCH_VAR_R; i++)
			bb_temp[i] = (bb[i>>5]>>(i&0x1F)) & 0x00000001 ;

		for( i=0; i<Parallel; i++ ) // 8bit
			bb_temp[iNX_BCH_VAR_R - Parallel + i] ^= (pData32[iii>>2]>>(i+(iii<<3))) & 0x00000001;

		for (i = 0; i < iNX_BCH_VAR_R; i++)
		{
			Temp = 0;
			for (j = 0; j < iNX_BCH_VAR_R; j++)
				Temp ^= (bb_temp[j] * T_G_R[i][j]);

			if(Temp)
				bb[i>>5] |=  Temp<<(i&0x1F);
			else
				bb[i>>5] &=  ~(1<<(i&0x1F));
		}
	}

	iii = ((iNX_BCH_VAR_R+(32-1)) >>5);

	for(i=0; i<iii; i++)
	{
		Temp = 0;
		for(j=0; j<32; j++)
		{
			Temp |= ((bb[i]>>(31-j))&0x00000001)<<j;	// reverse each bit every 32 bit
		}
		bb[i] = Temp;
	}

	for(i=0; i<iii; i++)
	{
		Temp = 0;
		for(j=0; j<32; j+=8)
		{
			Temp |= ((bb[i]>>(24-j))&0x000000FF)<<j;	// reverse endian
		}
		bb[i]= Temp;
	}

	for (j = 0; j < iii; j++)
	{
		*pECC++ = bb[j];
	}
}


int	MakeECCChunk (unsigned int *pdwSrcAddr, unsigned int *pdwDstAddr, int SrcSize)
{
	int iSector, iSecCount;

	iSecCount = (SrcSize + iSectorSize-1) / iSectorSize;

	if (iSecCount > 7)	// only use 8 pages per block
	{
		printf ("MakeECCChuck : Error - size(%d) must be less than or equal to %d\n",
				SrcSize, 7*iSectorSize );	// only use 8 pages per block
		return 0;
	}

	memset ((void *)pdwDstAddr, 0, iSectorSize*8);	// only use 8 pages per block

	for (iSector=0 ; iSector < iSecCount ; iSector++)
	{
		memcpy ((void *)(pdwDstAddr+((iSector+1)*(iSectorSize/4))), (const void *)(pdwSrcAddr+(iSector*(iSectorSize/4))), iSectorSize);
		parallel_encode_bch ((pdwDstAddr+((iSector+1)*(iSectorSize/4))), pdwDstAddr+((iSector+1)*(iSectorSize/8/4)));

		printf (".");
	}

	printf ("\n");
	return (iSecCount+1) * iSectorSize;
}

