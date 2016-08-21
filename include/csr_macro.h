/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_macro.h
 *  \brief defines and functions for csr_macro.h
 *
 *   This file contains the function and definations for csr_macro.h
 */
 
/*****************************************************************************/

#ifndef CSR_MACRO_H__
#define CSR_MACRO_H__


#include "csr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*/
/* Bits - intended to operate on CsrUint32 values */
/*------------------------------------------------------------------*/
#define CSR_MASK_IS_SET(val, mask) (((val) & (mask)) == (mask))
#define CSR_MASK_IS_UNSET(val, mask) ((((val) & (mask)) ^ mask) == (mask))
#define CSR_MASK_SET(val, mask)    ((val) |= (mask))
#define CSR_MASK_UNSET(val, mask)  ((val) = ((val) ^ (mask)) & (val)) /* Unsets the bits in val that are set in mask */
#define CSR_BIT_IS_SET(val, bit)   ((CsrBool) ((((val) & (1UL << (bit))) != 0)))
#define CSR_BIT_SET(val, bit)      ((val) |= (1UL << (bit)))
#define CSR_BIT_UNSET(val, bit)    ((val) &= ~(1UL << (bit)))
#define CSR_BIT_TOGGLE(val, bit)   ((val) ^= (1UL << (bit)))

/*------------------------------------------------------------------*/
/* Endian conversion */
/*------------------------------------------------------------------*/
#define CSR_GET_UINT8(ptr) (((CsrUint8 *) (ptr))[0])
#define CSR_GET_UINT16_FROM_LITTLE_ENDIAN(ptr)    \
    (((CsrUint16) ((CsrUint8 *) (ptr))[0]) |      \
    ((CsrUint16) ((CsrUint8 *) (ptr))[1]) << 8)

#define CSR_GET_UINT24_FROM_LITTLE_ENDIAN(ptr)    \
    (((CsrUint24) ((CsrUint8 *) (ptr))[0]) |      \
    ((CsrUint24) ((CsrUint8 *) (ptr))[1]) << 8 |  \
    ((CsrUint24) ((CsrUint8 *) (ptr))[2]) << 16)

#define CSR_GET_UINT32_FROM_LITTLE_ENDIAN(ptr)    \
    (((CsrUint32) ((CsrUint8 *) (ptr))[0]) |      \
    ((CsrUint32) ((CsrUint8 *) (ptr))[1]) << 8 |  \
    ((CsrUint32) ((CsrUint8 *) (ptr))[2]) << 16 | \
    ((CsrUint32) ((CsrUint8 *) (ptr))[3]) << 24)

#define CSR_GET_UINT16_FROM_BIG_ENDIAN(ptr)                              \
    (((CsrUint16) ((CsrUint8 *) (ptr))[1]) |                             \
    ((CsrUint16) ((CsrUint8 *) (ptr))[0]) << 8)

#define CSR_GET_UINT24_FROM_BIG_ENDIAN(ptr)                              \
    (((CsrUint24) ((CsrUint8 *) (ptr))[2]) |                             \
    ((CsrUint24) ((CsrUint8 *) (ptr))[1]) << 8 |                         \
    ((CsrUint24) ((CsrUint8 *) (ptr))[0]) << 16)

#define CSR_GET_UINT32_FROM_BIG_ENDIAN(ptr)                              \
    (((CsrUint32) ((CsrUint8 *) (ptr))[3]) |                             \
    ((CsrUint32) ((CsrUint8 *) (ptr))[2]) << 8 |                         \
    ((CsrUint32) ((CsrUint8 *) (ptr))[1]) << 16 |                        \
    ((CsrUint32) ((CsrUint8 *) (ptr))[0]) << 24)


#define CSR_COPY_UINT8(uint, ptr)                          \
do \
{  \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) ((uint) & 0xFF)); \
} while(0)

#define CSR_COPY_UINT16_TO_LITTLE_ENDIAN(uint, ptr)            \
do \
{  \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) ((uint) & 0x00FF));  \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) ((uint) >> 8));      \
} while(0)

#define CSR_COPY_UINT24_TO_LITTLE_ENDIAN(uint, ptr)                      \
do \
{  \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) ((uint) & 0x000000FF));        \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) (((uint) >> 8) & 0x000000FF)); \
    ((CsrUint8 *) (ptr))[2] = ((CsrUint8) (((uint) >> 16) & 0x000000FF));\
} while(0)

#define CSR_COPY_UINT32_TO_LITTLE_ENDIAN(uint, ptr)                      \
do \
{  \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) ((uint) & 0x000000FF));        \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) (((uint) >> 8) & 0x000000FF)); \
    ((CsrUint8 *) (ptr))[2] = ((CsrUint8) (((uint) >> 16) & 0x000000FF));\
    ((CsrUint8 *) (ptr))[3] = ((CsrUint8) (((uint) >> 24) & 0x000000FF));\
} while(0)


#define CSR_COPY_UINT16_TO_BIG_ENDIAN(uint, ptr)                         \
do \
{  \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) ((uint) & 0x00FF));            \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) ((uint) >> 8));                \
} while(0)

#define CSR_COPY_UINT24_TO_BIG_ENDIAN(uint, ptr)                         \
do \
{  \
    ((CsrUint8 *) (ptr))[2] = ((CsrUint8) ((uint) & 0x000000FF));        \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) (((uint) >> 8) & 0x000000FF)); \
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) (((uint) >> 16) & 0x000000FF));\
} while(0)

#define CSR_COPY_UINT32_TO_BIG_ENDIAN(uint, ptr)                         \
do \
{  \
    ((CsrUint8 *) (ptr))[3] = ((CsrUint8) ((uint) & 0x000000FF));        \
    ((CsrUint8 *) (ptr))[2] = ((CsrUint8) (((uint) >> 8) & 0x000000FF)); \
    ((CsrUint8 *) (ptr))[1] = ((CsrUint8) (((uint) >> 16) & 0x000000FF));\
    ((CsrUint8 *) (ptr))[0] = ((CsrUint8) (((uint) >> 24) & 0x000000FF));\
} while(0)


/*------------------------------------------------------------------*/
/* Model macros */
/*------------------------------------------------------------------*/
/* Macors to Write a data type value into buffer in LE order */
#define WRITE_CsrUint8( p, v, offset, len) CSR_COPY_UINT8(v, &p[offset]); offset++
#define WRITE_CsrUint16(p, v, offset, len) CSR_COPY_UINT16_TO_LITTLE_ENDIAN(v, &p[offset]); offset += 2
#define WRITE_CsrUint24(p, v, offset, len) CSR_COPY_UINT24_TO_LITTLE_ENDIAN(v, &p[offset]); offset += 3
#define WRITE_CsrUint32(p, v, offset, len) CSR_COPY_UINT32_TO_LITTLE_ENDIAN(v, &p[offset]); offset += 4
#define WRITE_CsrUint8Ptr(msg, v, offset, len) CsrMemCpy(msg + offset, v, len)

/* Macros to Read a data type value in LE order from a buffer */
#define READ_CsrUint8(v, msg, len)  v = CSR_GET_UINT8(msg); msg++
#define READ_CsrUint16(v, msg, len) v = CSR_GET_UINT16_FROM_LITTLE_ENDIAN(msg); msg += 2
#define READ_CsrUint24(v, msg, len) v = CSR_GET_UINT24_FROM_LITTLE_ENDIAN(msg); msg += 3
#define READ_CsrUint32(v, msg, len) v = CSR_GET_UINT32_FROM_LITTLE_ENDIAN(msg); msg += 4
#define READ_CsrUint8Ptr(v, msg, len) CsrMemCpy(v, msg, len); msg += len

#define WRITE_sensor_type_t WRITE_CsrUint16
#define WRITE_CsrInt8  WRITE_CsrUint8 
#define WRITE_CsrBool     WRITE_CsrUint8
#define WRITE_CsrInt16 WRITE_CsrUint16
#define WRITE_CsrInt24 WRITE_CsrUint24
#define WRITE_CsrInt32 WRITE_CsrUint32

#define READ_sensor_type_t READ_CsrUint16
#define READ_CsrInt8  READ_CsrUint8 
#define READ_CsrBool     READ_CsrUint8
#define READ_CsrInt16 READ_CsrUint16
#define READ_CsrInt24 READ_CsrUint24
#define READ_CsrInt32 READ_CsrUint32


/*------------------------------------------------------------------*/
/* XAP conversion macros */
/*------------------------------------------------------------------*/

#define CSR_LSB16(a) ((CsrUint8) ((a) & 0x00ff))
#define CSR_MSB16(b) ((CsrUint8) ((b) >> 8))

#define CSR_CONVERT_8_FROM_XAP(output, input) \
    (output) = ((CsrUint8) (input));(input) += 2

#define CSR_CONVERT_16_FROM_XAP(output, input) \
    (output) = (CsrUint16) ((((CsrUint16) (input)[1]) << 8) | \
                            ((CsrUint16) (input)[0]));(input) += 2

#define CSR_CONVERT_32_FROM_XAP(output, input) \
    (output) = (((CsrUint32) (input)[1]) << 24) | \
               (((CsrUint32) (input)[0]) << 16) | \
               (((CsrUint32) (input)[3]) << 8) | \
               ((CsrUint32) (input)[2]);input += 4

#define CSR_ADD_UINT8_TO_XAP(output, input) \
    (output)[0] = (input);  \
    (output)[1] = 0;(output) += 2

#define CSR_ADD_UINT16_TO_XAP(output, input) \
    (output)[0] = ((CsrUint8) ((input) & 0x00FF));  \
    (output)[1] = ((CsrUint8) ((input) >> 8));(output) += 2

#define CSR_ADD_UINT32_TO_XAP(output, input) \
    (output)[0] = ((CsrUint8) (((input) >> 16) & 0x00FF)); \
    (output)[1] = ((CsrUint8) ((input) >> 24)); \
    (output)[2] = ((CsrUint8) ((input) & 0x00FF)); \
    (output)[3] = ((CsrUint8) (((input) >> 8) & 0x00FF));(output) += 4

/*------------------------------------------------------------------*/
/* Misc */
/*------------------------------------------------------------------*/
#define CSRMAX(a, b)    (((a) > (b)) ? (a) : (b))
#define CSRMIN(a, b)    (((a) < (b)) ? (a) : (b))

/* Use this macro on unused local variables that cannot be removed (such as
   unused function parameters). This will quell warnings from certain compilers
   and static code analysis tools like Lint and Valgrind. */
#define CSR_UNUSED(x) ((void) (x))

#define CSR_TOUPPER(character)  (((character) >= 'a') && ((character) <= 'z') ? ((character) - 0x20) : (character))
#define CSR_TOLOWER(character)  (((character) >= 'A') && ((character) <= 'Z') ? ((character) + 0x20) : (character))
#define CSR_ARRAY_SIZE(x)       (sizeof(x) / sizeof(*(x)))

#ifdef __cplusplus
}
#endif

#endif
