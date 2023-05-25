#ifndef __GEN_NANDBOOTEC_h__
#define	__GEN_NANDBOOTEC_h__


#define MAX_2NDBOOT			(16384)
#define ROMBOOT_MAXPAGE		(4096)

#define NSIH_LEN			(512)
#define COPY_LEN			(8192)


/* UTIL MACRO */
#define __ALIGN(x, a)		__ALIGN_MASK(x, (a) - 1)
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))

#define ALIGN(x, a)		__ALIGN((x), (a))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define IS_POWER_OF_2(n)	((n) && ((n) & ((n)-1)) == 0)


//--------------------------------------------------------------------------
// BCH variables:
//--------------------------------------------------------------------------
//	k : number of information
//	m : dimension of Galois field.
//	t : number of error that can be corrected.
//	n : length of codeword = 2^m - 1
//	r : number of parity bit = m * t
//--------------------------------------------------------------------------
#define NX_BCH_SECTOR_MAX	1024
#define NX_BCH_VAR_K		(1024 * 8)
#define NX_BCH_VAR_M		(14)
#define NX_BCH_VAR_T		(60)		// 4 or 8 or 16

#define NX_BCH_VAR_N		(((1<<NX_BCH_VAR_M)-1))
#define NX_BCH_VAR_R		(NX_BCH_VAR_M * NX_BCH_VAR_T)

#define NX_BCH_VAR_TMAX		(60)
#define NX_BCH_VAR_RMAX		(NX_BCH_VAR_M * NX_BCH_VAR_TMAX)

#define NX_BCH_VAR_R32		((NX_BCH_VAR_R   +31)/32)
#define NX_BCH_VAR_RMAX32	((NX_BCH_VAR_RMAX+31)/32)

#define mm_max				14				/* Dimension of Galoise Field */
#define nn_max				32768			/* Length of codeword, n = 2**m - 1 */
#define tt_max				60				/* Number of errors that can be corrected */
#define kk_max				32768			/* Length of information bit, kk = nn - rr  */
#define rr_max				1024			/* Number of parity checks, rr = deg[g(x)] */
#define parallel_max		32				/* Number of parallel encoding/syndrome computations */


extern unsigned int iSectorSize;
extern unsigned int iNX_BCH_VAR_K;							// (512 * 8) or (1024 * 8)
extern unsigned int iNX_BCH_VAR_M;							// 13 or 14
extern unsigned int iNX_BCH_VAR_T;							// 24 or 60
extern unsigned int iNX_BCH_VAR_N;
extern unsigned int iNX_BCH_VAR_R;
extern unsigned int iNX_BCH_VAR_TMAX;
extern unsigned int iNX_BCH_VAR_RMAX;
extern unsigned int iNX_BCH_VAR_R32;
extern unsigned int iNX_BCH_VAR_RMAX32;
extern unsigned int iNX_BCH_OFFSET;


#ifdef __cplusplus
extern "C"
{
#endif

void gen_poly( void );
void generate_gf( void );
int	MakeECCChunk (unsigned int *pdwSrcAddr, unsigned int *pdwDstAddr, int SrcSize);

#ifdef __cplusplus
}
#endif


#endif	// __GEN_NANDBOOTEC_h__
