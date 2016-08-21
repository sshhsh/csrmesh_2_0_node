 /******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_types.h
 *  \brief CSRmesh library data types
 *
 *   This file contains the different data types used in CSRmesh library
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 */
 /*****************************************************************************/

#ifndef __CSR_TYPES_H__
#define __CSR_TYPES_H__

#if defined(CSR101x) || defined(CSR101x_A05)
#include <types.h>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#endif

/*! \addtogroup CSRmesh
 * @{
 */
 
#ifdef __cplusplus
extern "C" {
#endif

#undef TRUE
#undef FALSE

#define TRUE 1
#define FALSE 0

#if !defined(CSR101x) && !defined(CSR101x_A05)
/* Unsigned fixed width types */
typedef uint8_t CsrUint8;
typedef uint16_t CsrUint16;
typedef uint32_t CsrUint24;
typedef uint32_t CsrUint32;

/* Signed fixed width types */
typedef int8_t CsrInt8;
typedef int16_t CsrInt16;
typedef int32_t CsrInt32;

#define min(x, y)    (((x) < (y)) ? (x) : (y))
#else
/* Unsigned fixed width types */
typedef uint8 CsrUint8;
typedef uint16 CsrUint16;
typedef uint24 CsrUint24;
typedef uint32 CsrUint32;

/* Signed fixed width types */
typedef int8 CsrInt8;
typedef int16 CsrInt16;
typedef int32 CsrInt32;
#endif

/* String types */
typedef char CsrCharString;

#if defined(CSR101x) || defined(CSR101x_A05)
typedef uint16 CsrSize;
/* Boolean */
typedef bool CsrBool;
#else
typedef uint16_t CsrSize;
typedef unsigned char CsrBool;
#endif

#define DEFINE_STATIC(VAR_TYPE, VAR_NAME, VAR_INIT_VAL) static VAR_TYPE VAR_NAME = VAR_INIT_VAL
#define DEFINE_STATIC_CONST(VAR_TYPE, VAR_NAME, VAR_INIT_VAL) static const VAR_TYPE VAR_NAME = VAR_INIT_VAL
#define DEFINE_EXTERN(VAR_TYPE, VAR_NAME, VAR_INIT_VAL) extern VAR_TYPE VAR_NAME = VAR_INIT_VAL

#define DSTATIC static
#define DEXTERN extern

/* Enable the following define to enable selective logging in a debug disabled build */

/* #define CSR_MESH_CRITICAL_TEST_DEBUG */
#ifdef CSR_MESH_CRITICAL_TEST_DEBUG
#include <debug.h>
#include <uart.h>
#endif

#ifdef __cplusplus
}
#endif

/*!@} */
#endif /*__CSR_TYPES_H__ */
