/*
 * Copyright 1994-2011 The MathWorks, Inc.
 *
 * File: ext_conv.c     $Revision: 1.1.6.9 $
 *
 * Abstract:
 * Utility functions for ext_comm.c.
 */

#include <string.h>

#include "tmwtypes.h"
#include "mex.h"
#include "extsim.h"
#include "extutil.h"


/* Function: slCopyTwoBytes ====================================================
 * Abstract:
 *  Copy two byte elements from src to dst, reversing byte ordering if
 *  requested (e.g., 01 becomes 10).  Handles both aligned and unaligned
 *  sources and destinations.  If src and dst are the same, the function
 *  performs "in place" byte swapping.
 *
 *  ??? This function seems a bit over complicated.
 */
static void slCopyTwoBytes(
    void       *y,         /* Pointer to start of dst   */
    const void *x,	   /* Pointer to start of src   */
    const unsigned long n, /* Number elements in vector */
    boolean_T  swapBytes)
{
#define MAX_ELS (256)
    if (!swapBytes) {
        if (x != y) {
            (void)memcpy(y, x, n*2);
        } else {
            return;
        }
    } else {
        if (IS_ALIGNED(x,2) && IS_ALIGNED(y,2)) {
            char       *yChar        = (char *)y;
            const char *xChar        = (const char *)x;
            const boolean_T  xCharAligned4 = IS_ALIGNED(xChar,4);
            const boolean_T  yCharAligned4 = IS_ALIGNED(yChar,4);

            /*
             * See if we can optimize by doing two swaps at a time
             * within a 4 byte container.  To do this, the src and dst
             * (x and y) arrays must have the same aligment wrt a 4 byte
             * boundary.
             */



            if ((xCharAligned4 == yCharAligned4) && (n > 1)) {
                int tmpN = (int) n;

                /*
                 * Swap the first 2 bytes if address is not aligned on 4
                 * byte boundary.
                 */
                if (!xCharAligned4) {
                    const uint16_T * const p16Src = (const uint16_T *)xChar; /*lint !e826 pointer-to-pointer conversion (smaller area) */

                    uint16_T * const p16Dst = (uint16_T *)yChar; /*lint !e826 pointer-to-pointer conversion (smaller area) */

                    *p16Dst = (uint16_T)
                        ((((*p16Src) >> 8) & 0x00FF) | 
                         (((*p16Src) << 8) & 0xFF00));

                    xChar += 2;
                    yChar += 2;
                    tmpN--;
                }

                assert(IS_ALIGNED(xChar,4));
                assert(IS_ALIGNED(yChar,4));

                /* Swap 4-bytes at a time */
                if ((tmpN-2) >= 0) {
	            const uint32_T *p32Src = (const uint32_T *)xChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */
	            uint32_T *p32Dst = (uint32_T *)yChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */
	            const uint32_T mask1   = 0x00FF00FF;
	            const uint32_T mask2   = 0xFF00FF00;

                    while ((tmpN-2) >= 0) {
                        *p32Dst = (((*p32Src) >> 8) & mask1) |
                                  (((*p32Src) << 8) & mask2);
                        p32Src++;
                        p32Dst++;
                        tmpN -= 2;
                    }

                    xChar = (const char *)p32Src;
                    yChar = (char *)p32Dst;
                }

                assert(tmpN >= 0);

                /*
                 * Take care of last 2 bytes if needed.
                 */
                if (tmpN > 0) {
                    const uint16_T * const p16Src = (const uint16_T *)xChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */
                    uint16_T * const p16Dst = (uint16_T *)yChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */
                    
                    assert(tmpN == 1);

                    *p16Dst = (uint16_T)
                        ((((*p16Src) >> 8) & 0x00FF) | 
                         (((*p16Src) << 8) & 0xFF00));

                    tmpN--;
                }

                assert(tmpN == 0);
            } else {  /*lint !e438 Last value assigned to variable 'tmpN' not used */
                /*
                 * Not aligned for 4 bytes operations, so do a series of
                 * 2 byte operations.
                 */
                int      i;
                const uint16_T *p16Src = (const uint16_T *)xChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */
                uint16_T *p16Dst = (uint16_T *)yChar;/*lint !e826 pointer-to-pointer conversion (smaller area) */

                for (i=0; i<(int)n; i++) {
                    *p16Dst = (uint16_T)
                        ((((*p16Src) >> 8) & 0x00FF) | 
                         (((*p16Src) << 8) & 0xFF00));

                    p16Src++;
                    p16Dst++;
                }

            }
        } else {
            /* account for mis-alignment */
            int count = 0;
            const char *src = (const char *)x;
            char *dst = (char *)y;
        
            while(count < (int)n) {
                UINT16_T tmp[MAX_ELS];
                const int      nLeft   = (int) n - count;
                const unsigned long    nEls    = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;/*lint !e732 !e737 !e712 Loss of sign and precision in promotion from int to unsigned long */
                const unsigned long    nBytes  = (unsigned long) nEls * 2;

                /* get the elements properly aligned */
                (void)memcpy(tmp, src, nBytes);
            
                /* swap them (in place) */
                slCopyTwoBytes(tmp, tmp, nEls, swapBytes);
                
                /* copy to destination */
                (void)memcpy(dst, tmp, nBytes);

                src   += nBytes;
                dst   += nBytes;
                count += nEls; /*lint !e737 !e712 Loss of sign and precision in promotion from int to unsigned long */
            }
        }
    }
#undef MAX_ELS
} /* end slCopyTwoBytes */


/* Function: slCopyFourBytes ===================================================
 * Abstract:
 *  Copy four byte elements from src to dst, reversing byte ordering if
 *  requested (e.g., 0123 becomes 3210).  Handles both aligned and unaligned
 *  sources and destinations.  If src and dst are the same, the function
 *  performs "in place" byte swapping.
 */
static void slCopyFourBytes(
    void       *y,        /* Pointer to start of dst   */
    const void *x,	  /* Pointer to start of src   */
    const int  n,         /* Number elements in vector */
    const boolean_T  swapBytes)
{
#define MAX_ELS (256)

    if (!swapBytes) {
        if (x != y) {
            (void)memcpy(y, x, (unsigned long)n * 4); /*lint !e571 Suspicious cast */
        } else {
            return;
        }
    } else {
        int i;
    
        if (IS_ALIGNED(x,4) && IS_ALIGNED(y,4)) {
            const UINT32_T *src = (const UINT32_T *)x;
            UINT32_T *dst = (UINT32_T *)y;
            for (i=0; i<n; i++, src++, dst++) {
                *dst =  ((*src >> 24) & 0xff)    |
                        ((*src >> 8) & 0xff00)   |
		        ((*src << 8) & 0xff0000) |
                        (*src << 24);
            }
        } else {
            /* account for mis-alignment */
            int count = 0;
            const char *src = (const char *)x;
            char *dst = (char *)y;
        
            while(count < n) {
                UINT32_T tmp[MAX_ELS];
                UINT32_T *tmpPtr = tmp;
                const int      nLeft   = n - count;
                const int      nEls    = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;
                const int      nBytes  = nEls * 4;

                /* get the elements properly aligned */
                (void)memcpy(tmp, src, (unsigned long) nBytes); /*lint !e571 Suspicious cast  */
            
                /* swap them (in place) */
                for (i=0; i<nEls; i++, tmpPtr++) {
                    *tmpPtr =  ((*tmpPtr >> 24) & 0xff)    |
                               ((*tmpPtr >> 8) & 0xff00)   |
		               ((*tmpPtr << 8) & 0xff0000) |
                               (*tmpPtr << 24);
                }

                /* copy to destination */
                (void)memcpy(dst, tmp, (unsigned long) nBytes); /*lint !e571 Suspicious cast  */

                src   += nBytes;
                dst   += nBytes;
                count += nEls;
            }
        }
    }
#undef MAX_ELS
} /* end slCopyFourBytes */


/* Function: slCopyEightBytes ==================================================
 * Abstract:
 *  Copy eight byte elements from src to dst, reversing byte ordering if
 *  requested (e.g., 01234567 becomes 76543210).  Handles both aligned
 *  and unaligned sources and destinations.  If src and dst are the same, the
 *  function performs "in place" byte swapping.
 */
static void slCopyEightBytes(
    void       *y,       /* Pointer to start of dst   */
    const void *x,	 /* Pointer to start of src   */
    const int  n,        /* Number elements in vector */
    const bool       swapBytes)
{
#define MAX_ELS (256)

    if (!swapBytes) {
        if (x != y) {
            (void)memcpy(y, x, (unsigned long) n*8);/*lint !e571 Suspicious cast  */
        } else {
            return;
        }
    } else {
        int i;

        if (IS_ALIGNED(x,8) && IS_ALIGNED(y,8)) {
            const UINT32_T *src = (const UINT32_T *)x;
            UINT32_T *dst = (UINT32_T *)y;
            for (i=0; i<n; i++, src += 2) {
                const UINT32_T r1 = *src;
                const UINT32_T r2 = *(src+1);

                *dst =  ((r2 >> 24) & 0xff)    |
                        ((r2 >> 8) & 0xff00)   |
		        ((r2 << 8) & 0xff0000) |
                        (r2 << 24);
                dst++;

                *dst =  ((r1 >> 24) & 0xff)     |
                        ((r1 >> 8)  & 0xff00)   |
		        ((r1 << 8)  & 0xff0000) |
                        (r1 << 24);
                dst++;
            }
        } else {
            /* account for mis-alignment */
            int count = 0;
            const char *src = (const char *)x;
            char *dst = (char *)y;
        
            while(count < n) {
                UINT32_T tmp[MAX_ELS*2];
                UINT32_T *tmpPtr = tmp;
                const int      nLeft   = n - count;
                const int      nEls    = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;
                const int      nBytes  = nEls * 8;

                /* get the elements properly aligned */
                (void)memcpy(tmp, src, (unsigned long) nBytes); /*lint !e571 Suspicious cast  */
            
                /* swap them (in place) */
                for (i=0; i<nEls; i++) {
                    const UINT32_T r1 = *tmpPtr;
                    const UINT32_T r2 = *(tmpPtr+1);

                    *tmpPtr = ((r2 >> 24) & 0xff)     |
                              ((r2 >> 8)  & 0xff00)   |
		              ((r2 << 8)  & 0xff0000) |
                              (r2 << 24);
                    tmpPtr++;

                    *tmpPtr = ((r1 >> 24) & 0xff)     |
                              ((r1 >> 8)  & 0xff00)   |
		              ((r1 << 8)  & 0xff0000) |
                              (r1 << 24);
                    tmpPtr++;
                }

                /* copy to destination */
                (void)memcpy(dst, tmp, (unsigned long) nBytes); /*lint !e571 Suspicious cast  */

                src   += nBytes;
                dst   += nBytes;
                count += nEls;
            }
        }
    }
#undef MAX_ELS
} /* end slCopyEightBytes */


/* Function: slCopyNBytes ======================================================
 * Abstract:
 *  Well, not really copy N bytes, although having an optimized N byte 
 *  copier/swapper would be a really good idea.  Currently, this function
 *  is just a dispatcher for pre-existing byte swapper/copier functions
 *  (2, 4, 8 byte swaps).
 */
static void slCopyNBytes(
    void       *y,        /* Pointer to start of dst   */
    const void * const x, /* Pointer to start of src   */
    const int  n,         /* Number elements in vector */
    boolean_T  swapBytes, /* true to swap              */
    const int  elSize)    /* size of elements in array */
{
    switch(elSize) {

    case 1:
      (void)memcpy(y, x, (unsigned long) n);/*lint !e571 Suspicious cast  */
        break;

    case 2:
      slCopyTwoBytes(y, x, (unsigned long) n, swapBytes);/*lint !e571 Suspicious cast  */
        break;

    case 4:
        slCopyFourBytes(y, x, n, swapBytes);
        break;

    case 8:
        slCopyEightBytes(y, x, n, swapBytes);
        break;
    
    default:
        /*
         * This implementation of ext_convert only supplies byte swapping
         * routines for arrays of 2, 4 and 8 byte elements.
         */
        assert(FALSE);
        break;
    } /* end switch */
} /* end slCopyNBytes */

#if defined (EXTMODE_DOUBLE_SWAP_4_BYTE_WORDS)
/* Function: DoubleSwap4ByteWords ======================================
 * Abstract:
 * Swap high 4 bytes and low 4 bytes. For example, LEGO NXT has big
 * endian word ordering for doubles, so we need to swap the high and
 * low 4 bytes to interpret doubles correctly on the host.
 */
static void DoubleSwap4ByteWords(
        const char * const srcPtr,
        char * const dstPtr)
    {        
        *dstPtr       = *(srcPtr + 4);
        *(dstPtr + 1) = *(srcPtr + 5);
        *(dstPtr + 2) = *(srcPtr + 6);
        *(dstPtr + 3) = *(srcPtr + 7);
        *(dstPtr + 4) = *srcPtr;
        *(dstPtr + 5) = *(srcPtr + 1);
        *(dstPtr + 6) = *(srcPtr + 2);
        *(dstPtr + 7) = *(srcPtr + 3);
    } /* end DoubleSwap4ByteWords */
#endif

/* Function: Double_HostToTarget ===============================================
 * Abstract:
 *  Convert Simulink (hosts) double value (8 bytes) to target real_T value.
 *  No assumptions may be made about the alignment of the dst ptr.
 *  The src pointer is aligned for type double.  As implemented, this function
 *  supports only 32 and 64 bit target real values (ieee).
 */
static void Double_HostToTarget(
    ExternalSim   *ES,/*lint !e952 Parameter 'ES' could be declared const */
    char          *dst,
    const void    * const voidSrc,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    const double    * const src      = (const double *)voidSrc;

    const int sizeofTargetDouble = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                esGetHostBytesPerTargetByte(ES);

    switch(sizeofTargetDouble) {
    case 4:
    {
        int  i;
        char *dstPtr = dst;

        for (i=0; i<n; i++) {
	    const float tmpFloat = (float)src[i];
            (void)memcpy(dstPtr, &tmpFloat, 4UL);
            dstPtr += 4;
        }

        if (swapBytes) {
            slCopyFourBytes(dst, dst, n, swapBytes);
        }
        break;
    }

    case 8:
        slCopyEightBytes(dst, src, n, swapBytes);
        break;

    default:
        /* This implementations supports 64 and 32 bit reals only. */
        assert(FALSE);
        break;
    } /* end switch */
    
#if defined (EXTMODE_DOUBLE_SWAP_4_BYTE_WORDS)
    {
        DoubleSwap4ByteWords((const char * const) src, (char * const) dst);        
    }  
#endif

} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Double_HostToTarget */ 


/* Function: Double_TargetToHost ===============================================
 * Abstract:
 *  Convert target real_T value to host double value (8 bytes).  No assumptions
 *  may be made about the alignment of the src ptr.  The dst pointer is aligned
 *  for type double.  As implemented, this function supports only 32 and 64 bit
 *  target real values (ieee).
 */
static void Double_TargetToHost(
    ExternalSim   *ES,
    void          *voidDst,
    const char    *src,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    double    * const dst      = (double *)voidDst;
#   define    MAX_ELS     (1024)

    const int sizeofTargetDouble = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                esGetHostBytesPerTargetByte(ES);

    switch(sizeofTargetDouble) {
    case 4:
        if (swapBytes || !IS_ALIGNED(src,4)) {
            int  count = 0;
            while(count < n) {
                int   i;
                float tmp[MAX_ELS];
                const int   nLeft  = n - count;
                const int   nEls   = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;
                const int   nBytes = nEls * 4;

                /* get the floats properly aligned and byte swapped */
                slCopyFourBytes(tmp, src, nEls, swapBytes);

                for (i=0; i<nEls; i++) {
                    dst[count++] = (double)tmp[i];
                }
                src += nBytes;
            }
        } else {
            int   i;
            const float *fptr = (const float *)src; /*lint !e826 pointer-to-pointer conversion (smaller area) */

            for (i=0; i<n; i++) {
                dst[i] = (double)(*fptr++);
            }
        }
        break;

    case 8:
        slCopyEightBytes(dst, src, n, swapBytes);
        break;

    default:
        /* This implementations supports 64 and 32 bit reals only. */
        assert(FALSE);
        break;
    } /* end switch */
#undef MAX_ELS

#if defined (EXTMODE_DOUBLE_SWAP_4_BYTE_WORDS)
    {
        DoubleSwap4ByteWords((const char * const) src, (char * const) dst);        
    }  
#endif

} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Double_TargetToHost */


/* Function: Double_TargetToHost_IntOnly =======================================
 * Abstract:
 *  Convert target real_T value to host double value (8 bytes).  No assumptions
 *  may be made about the alignment of the src ptr.  The dst pointer is aligned
 *  for type double.
 *
 *  This function is used when the target has been built with the integer only
 *  option enabled.  In this case, the real_T datatype has been redefined to
 *  an integer type.  Therefore, any value being passed from the target to the
 *  host that would normally be considered a floating value, is now treated as
 *  an integer value.
 *
 *  As implemented, this function supports only 32 and 64 bit target
 *  integer values.
 */
static void Double_TargetToHost_IntOnly(
    ExternalSim   *ES,
    void          *voidDst,
    const char    *src,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    double    * const dst      = (double *) voidDst; 
#   define    MAX_ELS     (1024)

    const int sizeofTargetDouble = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                esGetHostBytesPerTargetByte(ES);

    switch(sizeofTargetDouble) {
    case 4:
        if (swapBytes || !IS_ALIGNED(src,4)) {
            int  count = 0;
            while(count < n) {
                int   i;
                INT32_T tmp[MAX_ELS];
                const int   nLeft  = n - count;
                const int   nEls   = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;
                const int   nBytes = nEls * 4;

                /* get the ints properly aligned and byte swapped */
                slCopyFourBytes(tmp, src, nEls, swapBytes);

                for (i=0; i<nEls; i++) {
                    dst[count++] = (INT32_T)tmp[i];
                }
                src += nBytes;
            }
        } else {
            int   i;
            const INT32_T *iptr = (const INT32_T *)src;/*lint !e826 pointer-to-pointer conversion (smaller area) */

            for (i=0; i<n; i++) {
                dst[i] = (INT32_T)(*iptr++);
            }
        }
        break;

    default:
        /* This implementations supports 32 bit reals only. */
        assert(FALSE);
        break;
    } /* end switch */
#undef MAX_ELS
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Double_TargetToHost_IntOnly */


/* Function: Double_HostToTarget_IntOnly =======================================
 * Abstract:
 *  Convert Simulink (hosts) double value (8 bytes) to target real_T value.
 *  No assumptions may be made about the alignment of the dst ptr.
 *  The src pointer is aligned for type double.
 *
 *  This function is used when the target has been built with the integer only
 *  option enabled.  In this case, the real_T datatype has been redefined to
 *  an integer type.  Therefore, any value being passed from the host to the
 *  target that would normally be considered a floating value, is now treated as
 *  an integer value.
 *
 *  As implemented, this function supports only 32 and 64 bit target
 *  integer values.
 */
static void Double_HostToTarget_IntOnly(
    ExternalSim   *ES,
    char          *dst,
    const void    * const voidSrc,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    const double    * const src      = (const double *)voidSrc;

    const int sizeofTargetDouble = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                esGetHostBytesPerTargetByte(ES);

    switch(sizeofTargetDouble) {
    case 4:
    {
        int  i;
        char *dstPtr = dst;

        for (i=0; i<n; i++) {
	    const INT32_T tmpInt = (INT32_T)src[i];
            (void)memcpy(dstPtr, &tmpInt, 4UL);
            dstPtr += 4;
        }

        if (swapBytes) {
            slCopyFourBytes(dst, dst, n, swapBytes);
        }
        break;
    }

    default:
        /* This implementations supports 32 bit reals only. */
        assert(FALSE);
        break;
    } /* end switch */
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Double_HostToTarget_IntOnly */


/* Function: Bool_HostToTarget =================================================
 * Abstract:
 *  Convert Simulink (hosts) bool value (uint8_T) to target boolean_T value.
 *  No assumptions may be made about the alignment of the dst ptr.
 *  The src pointer is aligned for type uin8_T.  As implemented, this function
 *  supports either uint8_T boolean values on the target or uint32_T booleans
 *  on the target (for dsps that support only 32-bit words).
 */
static void Bool_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const voidSrc,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const boolean_T     swapBytes = esGetSwapBytes(ES);
    const uint8_T * const src      = (const uint8_T *)voidSrc;

    const int sizeofTargetBool = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                  esGetHostBytesPerTargetByte(ES);

    switch(sizeofTargetBool) {

    case 1:
        /* 
         * Since we assume uint8_t on both target and host, we have no byte
         * swapping issues.  This is just a straight copy.
         */
      (void)memcpy(dst, src, (unsigned long) n); /*lint !e571 Suspicious cast  */
        break;

    case 4:
    {
        int  i;
        char *dstPtr = dst;

        for (i=0; i<n; i++) {
	    const uint32_T tmp = (uint32_T)src[i];
            (void)memcpy(dstPtr, &tmp, 4UL);
            dstPtr += 4;
        }

        if (swapBytes) {
            slCopyFourBytes(dst, dst, n, swapBytes);
        }
        break;
    }

    default:
        /* This implementation supports only 8 and 32 bit bools on target */
        assert(FALSE);
        break;
    } /* end switch */
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Bool_HostToTarget */


/* Function: Bool_TargetToHost =================================================
 * Abstract:
 *  Convert target bool value to host bool value (uint8_t).  No assumptions may
 *  be made about the alignment of the src ptr.  The dst pointer is aligned for
 *  type uin8_T.  As implemented, this function supports either uint8_T boolean
 *  values on the target or uint32_T booleans on the target (for dsps that
 *  support only 32-bit words).
 */
static void Bool_TargetToHost(
    ExternalSim   *ES,
    void          *voidDst,
    const char    *src,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
#   define    MAX_ELS     (1024)
    const boolean_T swapBytes = esGetSwapBytes(ES);
    uint8_T   * const dst      = (uint8_T *)voidDst;

    const int  sizeofTargetBool = esGetSizeOfTargetDataTypeFcn(ES)(ES, dType) *
                                  esGetHostBytesPerTargetByte(ES);

    switch (sizeofTargetBool) {

    case 1:
        /* 
         * Since we assume uint8_t on both target and host, we have no byte
         * swapping issues.  This is just a straight copy.
         */
      (void)memcpy(dst, src, (unsigned long) n); /*lint !e571 Suspicious cast  */
        break;

    case 4:
        if (swapBytes || !IS_ALIGNED(src,4)) {
            int  count = 0;
            while(count < n) {
                int      i;
                uint32_T tmp[MAX_ELS];
                const int      nLeft  = n - count;
                const int      nEls   = (MAX_ELS >= nLeft) ? nLeft : MAX_ELS;
                const int      nBytes = nEls * 4;

                /* get the uint32_T's properly aligned and byte swapped */
                slCopyFourBytes(tmp, src, nEls, swapBytes);

                for (i=0; i<nEls; i++) {
                    dst[count++] = (uint8_T)tmp[i];
                }
                src += nBytes;
            }
        } else {
            int      i;
            const uint32_T *tmp = (const uint32_T *)src;/*lint !e826 pointer-to-pointer conversion (smaller area) */

            for (i=0; i<n; i++) {
                dst[i] = (uint8_T)(*tmp++);
            }
        }
        break;

    default:
        /* This implementation supports only 8 and 32 bit bools on target */
        assert(FALSE);
        break;
    } /* end switch */
#undef MAX_ELS
}/*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*/ /* end Bool_TargetToHost */


/* Function: Generic_HostToTarget ==============================================
 * Abstract:
 *  Convert generic data type from host to target format.  This function
 *  may be used with any data type where the number of bits is known to
 *  be the same on both host and target (e.g., int32_T, uint16_t, etc).
 *  It simply copies the correct number of bits from target to host performing
 *  byte swapping if required.  If any other conversion is required, then
 *  a custom HostToTarget function must be used.
 */
static void Generic_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const src,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const int       dTypeSize = esGetSizeOfDataTypeFcn(ES)(ES, dType);
    const boolean_T swapBytes = esGetSwapBytes(ES);
   
    slCopyNBytes(dst, src, n, swapBytes, dTypeSize);
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Generic_HostToTarget */


/* Function: Generic_TargetToHost ==============================================
 * Abstract:
 *  Convert generic data type from target to host format.  This function
 *  may be used with any data type where the number of bits is known to
 *  be the same on both host and target (e.g., int32_T, uint16_t, etc).
 *  It simply copies the correct number of bits from host to target performing
 *  byte swapping if required.  If any other conversion is required, then
 *  a custom TargetToHost function must be used.
 */
static void Generic_TargetToHost(
    ExternalSim  *ES,
    void         *dst,
    const char   * const src,
    const int    n,
    const int    dType) /* internal Simulink data type id */
{
    const int       dTypeSize = esGetSizeOfDataTypeFcn(ES)(ES, dType);
    const boolean_T swapBytes = esGetSwapBytes(ES);

    slCopyNBytes(dst, src, n, swapBytes, dTypeSize);
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Generic_TargetToHost */


/* Function: Int33Plus_HostToTarget ============================================
 * Abstract:
 *  Convert 33-bit or greater data type from host to target format.  This
 *  function may be used with any data type where the number of bits is known
 *  to be the same on both host and target and the size of the data type is
 *  33-bits or greater (e.g., uint33_T, int64_T, uint128_T, etc).  For types
 *  which fit inside a 64-bit container, the implementation is either a big
 *  long (native 64-bit integer) or multiword (an array of two native 32-bit
 *  integers).  For types greater than 64 bits, the implementation is multiword
 *  where the chunk size could be 32 or 64 bits depending on the native size
 *  of the processor.  This function copies the correct number of bits from
 *  host to target performing byte swapping if required.  If any other
 *  conversion is required, then a custom HostToTarget function must be used.
 *
 * NOTE:
 *  Input argument "n" is the number of elements to copy, where each element
 *  is dTypeSize bytes wide.  For multiword types, the type itself is made up
 *  of an array of smaller elements.  So we adjust the value of "n" to account
 *  for these extra elements when copying the data.
 */
static void Int33Plus_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const src,
    const int     n,
    const int     dType) /* internal Simulink data type id */
{
    const int       dTypeSize         = esGetSizeOfDataTypeFcn(ES)(ES, dType);
    const boolean_T swapBytes         = esGetSwapBytes(ES);
    const int       targetMWChunkSize = esGetTargetMWChunkSize(ES); /*lint !e713 Loss of precision (initialization) (unsigned int to int) *//* in bytes */
    const int       new_n             = n * (dTypeSize/targetMWChunkSize);

    slCopyNBytes(dst, src, new_n, swapBytes, targetMWChunkSize);

} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Int33Plus_HostToTarget */


/* Function: Int33Plus_TargetToHost ============================================
 * Abstract:
 *  Convert 33-bit or greater data type from target to host format.  This
 *  function may be used with any data type where the number of bits is known
 *  to be the same on both host and target and the size of the data type is
 *  33-bits or greater (e.g., uint33_T, int64_T, uint128_T, etc).  For types
 *  which fit inside a 64-bit container, the implementation is either a big
 *  long (native 64-bit integer) or multiword (an array of two native 32-bit
 *  integers).  For types greater than 64 bits, the implementation is multiword
 *  where the chunk size could be 32 or 64 bits depending on the native size
 *  of the processor.  This function copies the correct number of bits from
 *  target to host performing byte swapping if required.  If any other
 *  conversion is required, then a custom TargetToHost function must be used.
 *
 * NOTE:
 *  Input argument "n" is the number of elements to copy, where each element
 *  is dTypeSize bytes wide.  For multiword types, the type itself is made up
 *  of an array of smaller elements.  So we adjust the value of "n" to account
 *  for these extra elements when copying the data.
 */
static void Int33Plus_TargetToHost(
    ExternalSim  *ES,
    void         *dst,
    const char   * const src,
    const int    n,
    const int    dType) /* internal Simulink data type id */
{
    const int       dTypeSize        = esGetSizeOfDataTypeFcn(ES)(ES, dType);
    const boolean_T swapBytes        = esGetSwapBytes(ES);
    const int       hostMWChunkSize  = (int) esGetHostMWChunkSize(ES); /* in bytes */
    const int       new_n            = n * (dTypeSize/hostMWChunkSize);

    slCopyNBytes(dst, src, new_n, swapBytes, hostMWChunkSize);

} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Int33Plus_TargetToHost */

/* Function: Copy32BitsToTarget ================================================
 * Abstract:
 *  Copy 32 bits to the target.  It is assumed that the only conversion needed
 *  is bytes swapping (if needed) (e.g., uint32, int32).  Note that this fcn
 *  does not rely on the Simulink Internal data type id.
 */
void Copy32BitsToTarget(
    ExternalSim  *ES,
    char         *dst,
    const void   * const src,
    const int    n)
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    slCopyNBytes(dst, src, n, swapBytes, 4);
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Copy32BitsToTarget */


/* Function: Copy32BitsFromTarget ==============================================
 * Abstract:
 *  Copy 32 bits from the target.  It is assumed that the only conversion needed
 *  is bytes swapping (if needed) (e.g., uint32, int32).  Note that this fcn
 *  does not rely on the Simulink Internal data type id.
 */
void Copy32BitsFromTarget(
    ExternalSim *ES,
    void        *dst,
    const char  * const src,
    const int    n)
{
    const boolean_T swapBytes = esGetSwapBytes(ES);
    slCopyNBytes(dst, src, n, swapBytes, 4);
} /*lint !e952 !e818 Parameter 'ES' could be declared const and as pointing to
   * const*//* end Copy32BitsFromTarget */


/* Function ====================================================================
 * Process the first of two EXT_CONNECT_RESPONSE packets from the target.
 * This packet consists of nothing but a packet header.  In this special
 * instance we interpret the size field as the number of bits in a target
 * byte (not always 8 - see TI compiler for C30 and C40).
 *
 * This function is responsible for deducing the endian format of the target,
 * validating that the number of bits per target byte and setting up pointers
 * to data conversion functions.
 *
 * NOTE: The caller must check that the error status is clear after calling
 *       this function (i.e., esIsErrorClear(ES)).
 */
void ProcessConnectResponse1(ExternalSim *ES, PktHeader *pktHdr)
{
    /*
     * Deduce the endian-ness of the target.
     */
    if (pktHdr->type == EXT_CONNECT_RESPONSE) { /*lint !e641 Converting enum 'ExtModeAction' to 'int'*/
        esSetSwapBytes(ES, FALSE);
    } else {
        const boolean_T swapBytes = TRUE;
        slCopyFourBytes(pktHdr, pktHdr, NUM_HDR_ELS, swapBytes);
        if (pktHdr->type != EXT_CONNECT_RESPONSE) {/*lint !e641 Converting enum 'ExtModeAction' to 'int'*/
            esSetError(ES, "Invalid EXT_CONNECT_RESPONSE packet.\n");
            return;
        }
        esSetSwapBytes(ES, swapBytes);
    }

    /*
     * Process bits per target byte.
     */
    {
        const int_T bitsPerTargetByte      = pktHdr->size; /*lint !e713 Loss of precision (initialization) (unsigned int to int) */
        const int_T hostBytesPerTargetByte = bitsPerTargetByte/8;
        
        assert(bitsPerTargetByte%8 == 0);
        esSetHostBytesPerTargetByte(ES, (uint8_T)hostBytesPerTargetByte);
    }

    /*
     * Set up fcn ptrs for data conversion - Simulink data types.
     */
    esSetDoubleTargetToHostFcn(ES, Double_TargetToHost);
    esSetDoubleHostToTargetFcn(ES, Double_HostToTarget);

    esSetSingleTargetToHostFcn(ES, Generic_TargetToHost); /* assume 32 bit */
    esSetSingleHostToTargetFcn(ES, Generic_HostToTarget); /* assume 32 bit */
    
    esSetInt8TargetToHostFcn(ES, Generic_TargetToHost);
    esSetInt8HostToTargetFcn(ES, Generic_HostToTarget);

    esSetUInt8TargetToHostFcn(ES, Generic_TargetToHost);
    esSetUInt8HostToTargetFcn(ES, Generic_HostToTarget);

    esSetInt16TargetToHostFcn(ES, Generic_TargetToHost);
    esSetInt16HostToTargetFcn(ES, Generic_HostToTarget);

    esSetUInt16TargetToHostFcn(ES, Generic_TargetToHost);
    esSetUInt16HostToTargetFcn(ES, Generic_HostToTarget);

    esSetInt32TargetToHostFcn(ES, Generic_TargetToHost);
    esSetInt32HostToTargetFcn(ES, Generic_HostToTarget);

    esSetUInt32TargetToHostFcn(ES, Generic_TargetToHost);
    esSetUInt32HostToTargetFcn(ES, Generic_HostToTarget);

    esSetInt33PlusTargetToHostFcn(ES, Int33Plus_TargetToHost);
    esSetInt33PlusHostToTargetFcn(ES, Int33Plus_HostToTarget);

    esSetBoolTargetToHostFcn(ES, Bool_TargetToHost);
    esSetBoolHostToTargetFcn(ES, Bool_HostToTarget);

    return;
} /* end ProcessConnectResponse1 */


/* Function ====================================================================
 *
 *   Process the data sizes information from the second EXT_CONNECT_RESPONSE 
 *   packets.  The data passed into this function is of the form:
 * 
 *           nDataTypes    - # of data types on target (uint32_T)
 *           dataTypeSizes - 1 per nDataTypes          (uint32_T[])
 *
 * NOTE: The host and target may not have the same number of data types
 *       for a variety of reasons.  One possibility is that the target
 *       code is not up to date with the Simulink model.  In this case,
 *       a checksum mismatch will be detected later on.  Another
 *       possibility is that the TLC code added some data types to the
 *       target and did not register them with Simulink.  In this case,
 *       there will not be a checksum mismatch, but it is OK to assume
 *       that the first N data types for the host and target are identical.
 *       We can copy over the first N sizes from the target and skip the
 *       rest (which do not exist in Simulink).  This should be OK because
 *       there can't be a signal in Simulink with one of these data types
 *       (if there was, the data type would have been registered in Simulink).
 *
 *       For all cases below, we copy the first N data type sizes from the
 *       target and write them into the first N data types for the host.
 *       If the host and target have different checksums, we will error out
 *       later.  If they have the same checksums, we should have enough data
 *       type information to continue with extmode.  It is not possible for
 *       the host to have more data types than the target because all data
 *       types registered in Simulink are written out to the rtw file for
 *       code generation.
 *
 *       Case 1:      Host and target have same data types which match up
 *                    perfectly together (i.e. they have the same data types
 *                    in the same order).
 *
 *                            host                 target
 *                          --------              --------
 *                          |   1  |              |   1  |
 *                          |      |              |      |
 *                          |   .  |              |   .  |
 *                          |   .  |   N Common   |   .  |
 *                          |   .  |  data types  |   .  |
 *                          |      |              |      |
 *                          |      |              |      |
 *                          |   N  |              |   N  |
 *                          --------              --------
 *
 *       Case 2:      Target has more data types than host, possibly because
 *                    the TLC code registered some data types which were not
 *                    registered in Simulink.
 *
 *                            host                 target
 *                          --------              --------
 *                          |   1  |              |   1  |
 *                          |      |              |      |
 *                          |   .  |              |   .  |
 *                          |   .  |   N Common   |   .  |
 *                          |   .  |  data types  |   .  |
 *                          |      |              |      |
 *                          |      |              |      |
 *                          |   N  |              |   N  |
 *                          --------              --------
 *                                                |  N+1 |
 *                                                |      |
 *                                                |      |
 *                                                |  N+M |
 *                                                --------
 *
 */
void ProcessTargetDataSizes(const ExternalSim * const ES, const uint32_T *bufPtr)
{
    uint32_T i;
    const uint32_T hostNDataTypes = esGetNumDataTypes(ES);
    const uint32_T targNDataTypes = *bufPtr++;
    const uint32_T copyNDataTypes = (hostNDataTypes < targNDataTypes) ? hostNDataTypes : targNDataTypes;

    for (i=0; i<copyNDataTypes; i++) {
        esSetDataTypeSize(ES, i, *bufPtr++);
    }
    return;
} /* end ProcessTargetDataSizes */


/* Function ====================================================================
 */
void InstallIntegerOnlyDoubleConversionRoutines(ExternalSim *ES)
{
    esSetDoubleTargetToHostFcn(ES, Double_TargetToHost_IntOnly);
    esSetDoubleHostToTargetFcn(ES, Double_HostToTarget_IntOnly);
} /* end InstallIntegerOnlyDoubleConversionRoutines */


/* [EOF] ext_convert.c */
