/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2019 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */
#include <sys/machine.h>
#ifdef CPU_CORE_RXV2
/*
 *	cpu_cntl.c (RXv2)
 *	CPU-Dependent Control
 */
#include "kernel.h"
#include "../../../sysdepend.h"

#include "cpu_task.h"


/* ------------------------------------------------------------------------ */
/*
 * Task independent status
 */
EXPORT	W	knl_taskindp = 0;

/* ------------------------------------------------------------------------ */
/* Temporal stack 
 *	used when 'dispatch_to_schedtsk' is called.
 */

Noinit(EXPORT UB knl_tmp_stack[TMP_STACK_SIZE]);

/* ------------------------------------------------------------------------ */
/*
 * Set task register contents (Used in tk_set_reg())
 */
EXPORT void knl_set_reg( CTXB *ctxb, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{
	SStackFrame	*ssp;
	INT		i;

	ssp = ctxb->ssp;

	if ( cregs != NULL ) {
		ssp = cregs->ssp;
	}

	if ( regs != NULL ) {
		for ( i = 0; i < 15; ++i ) {
			ssp->r[i] = regs->r[i];
		}
	}

	if ( eit != NULL ) {
		ssp->spc  = eit->pc;
		ssp->spsw = eit->psw;
	}

	if ( cregs != NULL ) {
		ctxb->ssp = cregs->ssp;
	}
}

/* ------------------------------------------------------------------------ */
/*
 * Get task register contents (Used in tk_get_reg())
 */
EXPORT void knl_get_reg( CTXB *ctxb, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{
	SStackFrame	*ssp;
	INT		i;

	ssp = ctxb->ssp;

	if ( regs != NULL ) {
		for ( i = 0; i < 15; ++i ) {
			regs->r[i] = ssp->r[i];
		}
	}

	if ( eit != NULL ) {
		eit->pc = ssp->spc;
		eit->psw = ssp->spsw;
	}

	if ( cregs != NULL ) {
		cregs->ssp = ctxb->ssp; 
	}
}

/* ------------------------------------------------------------------------ */
/*
 * Set Coprocessor registers contents (Used in tk_set_cpr())
 */
EXPORT ER knl_set_cpr( CTXB *ctxb, INT copno, CONST T_COPREGS *copregs )
{
	SStackFrame	*ssp;
	ER		ercd;

	ssp = ctxb->ssp;
	ercd = E_OK;

	switch(copno) {
	case COPNO_0:		/* copno 0: FPU */
#if	USE_FPU
		ssp->fpsw = copregs->cop0.fpsw;
#else
		ercd = E_PAR;
#endif
		break;

	case COPNO_1:			/* copno 1: DSP */
#if	USE_DSP
		ssp->acc0lo = copregs->cop1.acc0lo;
		ssp->acc0hi = copregs->cop1.acc0hi;
		ssp->acc0gu = copregs->cop1.acc0gu;
		ssp->acc1lo = copregs->cop1.acc1lo;
		ssp->acc1hi = copregs->cop1.acc1hi;
		ssp->acc1gu = copregs->cop1.acc1gu;
#else
		ercd = E_PAR;
#endif
		break;
	default:
		ercd = E_PAR;
	}

	return ercd;
}


/* ------------------------------------------------------------------------ */
/*
 * Get Coprocessor registers contents (Used in tk_get_cpr())
 */
EXPORT ER knl_get_cpr( CTXB *ctxb, INT copno, T_COPREGS *copregs)
{
	SStackFrame	*ssp;
	ER		ercd;

	ssp = ctxb->ssp;
	ercd = E_OK;

	switch (copno) {
	case COPNO_0:			/* copno 0: FPU */
#if	USE_FPU
		copregs->cop0.fpsw = ssp->fpsw;
#else
		ercd = E_PAR;
#endif
		break;

	case COPNO_1:			/* copno 1: DPS */
#if	USE_DSP
		copregs->cop1.acc0lo = ssp->acc0lo;
		copregs->cop1.acc0hi = ssp->acc0hi;
		copregs->cop1.acc0gu = ssp->acc0gu;
		copregs->cop1.acc1lo = ssp->acc1lo;
		copregs->cop1.acc1hi = ssp->acc1hi;
		copregs->cop1.acc1gu = ssp->acc1gu;
#else
		ercd = E_PAR;
#endif
		break;

	default:
		ercd = E_PAR;
		break;
	}

	return ercd;
}


/* ----------------------------------------------------------------------- */
/*
 *	Task dispatcher startup
 */
EXPORT void knl_force_dispatch( void )
{
	FP	fp = knl_dispatch_to_schedtsk;

	Asm("jsr %0" :: "r"(fp));	/* No return */
}

EXPORT void knl_dispatch( void )
{
	knl_dispatch_entry();
}

#endif /* CPU_CORE_RXV2 */