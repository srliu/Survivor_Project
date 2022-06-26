/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#include "../include/DSPLib.h"

#if defined(MSP_USE_LEA)

msp_status msp_offset_q15(const msp_offset_q15_params *params, const _q15 *src, _q15 *dst)
{
    uint16_t length;
    _q15 q15Offset;
    int16_t *offsetVector;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;
    
    /* Initialize the loop counter and offset. */
    length = params->length;
    q15Offset = params->offset;

#ifndef MSP_DISABLE_DIAGNOSTICS
    /* Check that length parameter is a multiple of two. */
    if (length & 1) {
        return MSP_SIZE_ERROR;
    }
    
    /* Check that the data arrays are aligned and in a valid memory segment. */
    if (!(MSP_LEA_VALID_ADDRESS(src, 4) &
          MSP_LEA_VALID_ADDRESS(dst, 4))) {
        return MSP_LEA_INVALID_ADDRESS;
    }

    /* Acquire lock for LEA module. */
    if (!msp_lea_acquireLock()) {
        return MSP_LEA_BUSY;
    }
#endif //MSP_DISABLE_DIAGNOSTICS

    /* Initialize LEA if it is not enabled. */
    if (!(LEAPMCTL & LEACMDEN)) {
        msp_lea_init();
    }
        
    /* Allocate MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams = (MSP_LEA_ADDMATRIX_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
        
    /* Allocate offset vector of length two. */
    offsetVector = (int16_t *)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    offsetVector[0] = q15Offset;
    offsetVector[1] = q15Offset;

    /* Set MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams->input2 = MSP_LEA_CONVERT_ADDRESS(offsetVector);
    leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);
    leaParams->vectorSize = length;
    leaParams->input1Offset = 1;
    leaParams->input2Offset = 0;
    leaParams->outputOffset = 1;

    /* Load source arguments to LEA. */
    LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
    LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

    /* Invoke the LEACMD__ADDMATRIX command. */
    msp_lea_invokeCommand(LEACMD__ADDMATRIX);

    /* Free MSP_LEA_ADDMATRIX_PARAMS structure and offset vector. */
    msp_lea_freeMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
    
    /* Set status flag. */
    status = MSP_SUCCESS;
        
#ifndef MSP_DISABLE_DIAGNOSTICS
    /* Check LEA interrupt flags for any errors. */
    if (msp_lea_ifg & LEACOVLIFG) {
        status = MSP_LEA_COMMAND_OVERFLOW;
    }
    else if (msp_lea_ifg & LEAOORIFG) {
        status = MSP_LEA_OUT_OF_RANGE;
    }
    else if (msp_lea_ifg & LEASDIIFG) {
        status = MSP_LEA_SCALAR_INCONSISTENCY;
    }
#endif

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();
    return status;
}

#else //MSP_USE_LEA

msp_status msp_offset_q15(const msp_offset_q15_params *params, const _q15 *src, _q15 *dst)
{
    uint16_t length;
    _q15 q15Offset;
    
    /* Initialize the loop counter and offset. */
    length = params->length;
    q15Offset = params->offset;

#ifndef MSP_DISABLE_DIAGNOSTICS
    /* Check that length parameter is a multiple of two. */
    if (length & 1) {
        return MSP_SIZE_ERROR;
    }
#endif //MSP_DISABLE_DIAGNOSTICS

    /* Loop through all vector elements. */
    while (length--) {
        /* Add offset to src with saturation and store result. */
        *dst++ = __saturated_add_q15(*src++, q15Offset);
    }

    return MSP_SUCCESS;
}

#endif //MSP_USE_LEA
