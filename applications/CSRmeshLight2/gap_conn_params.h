/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      gap_conn_params.h
 *
 *  DESCRIPTION
 *      MACROs for connection and advertisement parameter values
 *
 *****************************************************************************/

#ifndef __GAP_CONN_PARAMS_H__
#define __GAP_CONN_PARAMS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Maximum number of connection parameter update requests that can be send when 
 * connected
 */
#define MAX_NUM_CONN_PARAM_UPDATE_REQS       (4)

/* Brackets should not be used around the value of a macros used in db files. 
 * The parser which creates .c and .h files from .db file doesn't understand 
 * brackets and will raise syntax errors.
 */

/* Preferred connection parameter values should be within the range specified 
 * in the Bluetooth specification.
 */

/* Minimum and maximum connection interval in number of frames */
#define PREFERRED_MAX_CON_INTERVAL          96 /* 120 ms */
#define PREFERRED_MIN_CON_INTERVAL          72 /*  90 ms */

/* Slave latency in number of connection intervals */
#define PREFERRED_SLAVE_LATENCY             0x0000 /* 0 conn_intervals */

/* Supervision time-out (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define PREFERRED_SUPERVISION_TIMEOUT       0x0258 /* 6 seconds */


/* APPLE Compliant connection parameters */
/* Minimum and maximum connection interval in number of frames. */
#define APPLE_MAX_CON_INTERVAL              96 /* 120 ms */
#define APPLE_MIN_CON_INTERVAL              72 /*  90 ms */

/* Slave latency in number of connection intervals. */
#define APPLE_SLAVE_LATENCY                 0x0000 /* 0 conn_intervals. */

/* Supervision time-out (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define APPLE_SUPERVISION_TIMEOUT           0x0258 /* 6 seconds */


#endif /* __GAP_CONN_PARAMS_H__ */
