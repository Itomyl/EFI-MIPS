/*	$Id: ovbcopy.S,v 1.1.1.1 2006/06/29 06:43:26 cpu Exp $ */

#define	ABICALLS			# NO ABICALLS
#include "Mips.h"


/*
 * memcpy(to, from, len)
 * {ov}bcopy(from, to, len)
 */
LEAF(memcpy)
	.set	noreorder
	move	v0, a0			# swap from and to
	move	a0, a1
	move	a1, v0
ALEAF(bcopy)
ALEAF(ovbcopy)
#ifdef GODSONEV1
	.set	noreorder
	move	v0,a1
	bltu	a0,a1,2f
	nop
	addu	a0,a2
	addu	a1,a2
1:	bgez	a2,3f
	addiu	a2,-1
	lb	t0,-1(a0)
	sb	t0,-1(a1)
	addiu	a0,-1
	b	1b
	addiu	a1,-1
2:	bgez	a2,3f
	addiu	a2,-1
	lb	t0,0(a0)
	sb	t0,0(a1)
	addiu	a0,1
	b	2b
	addiu	a1,1
3:
	j	ra
	nop	
#else
	.set	noreorder
	addu	t0, a0, a2		# t0 = end of s1 region
	sltu	t1, a1, t0
	sltu	t2, a0, a1
	and	t1, t1, t2		# t1 = true if from < to < (from+len)
	beq	t1, zero, forward	# non overlapping, do forward copy
	slt	t2, a2, 12		# check for small copy

	ble	a2, zero, 2f
	addu	t1, a1, a2		# t1 = end of to region
1:
	lb	v1, -1(t0)		# copy bytes backwards,
	subu	t0, t0, 1		#   doesnt happen often so do slow way
	subu	t1, t1, 1
	bne	t0, a0, 1b
	sb	v1, 0(t1)
2:
	j	ra
	nop
forward:
	bne	t2, zero, smallcpy	# do a small bcopy
	xor	v1, a0, a1		# compare low two bits of addresses
	and	v1, v1, 3
	subu	a3, zero, a1		# compute # bytes to word align address
	beq	v1, zero, aligned	# addresses can be word aligned
	and	a3, a3, 3

	beq	a3, zero, 1f
	subu	a2, a2, a3		# subtract from remaining count
	LWHI	v1, 0(a0)		# get next 4 bytes (unaligned)
	LWLO	v1, 3(a0)
	addu	a0, a0, a3
	SWHI	v1, 0(a1)		# store 1, 2, or 3 bytes to align a1
	addu	a1, a1, a3
1:
	and	v1, a2, 3		# compute number of words left
	subu	a3, a2, v1
	move	a2, v1
	addu	a3, a3, a0		# compute ending address
2:
	LWHI	v1, 0(a0)		# copy words a0 unaligned, a1 aligned
	LWLO	v1, 3(a0)
	addu	a0, a0, 4
	sw	v1, 0(a1)
	addu	a1, a1, 4
	bne	a0, a3, 2b
	nop				# We have to do this mmu-bug.
	b	smallcpy
	nop
aligned:
	beq	a3, zero, 1f
	subu	a2, a2, a3		# subtract from remaining count
	LWHI	v1, 0(a0)		# copy 1, 2, or 3 bytes to align
	addu	a0, a0, a3
	SWHI	v1, 0(a1)
	addu	a1, a1, a3
1:
	and	v1, a2, 3		# compute number of whole words left
	subu	a3, a2, v1
	move	a2, v1
	addu	a3, a3, a0		# compute ending address
2:
	lw	v1, 0(a0)		# copy words
	addu	a0, a0, 4
	sw	v1, 0(a1)
	bne	a0, a3, 2b
	addu	a1, a1, 4
smallcpy:
	ble	a2, zero, 2f
	addu	a3, a2, a0		# compute ending address
1:
	lbu	v1, 0(a0)		# copy bytes
	addu	a0, a0, 1
	sb	v1, 0(a1)
	bne	a0, a3, 1b
	addu	a1, a1, 1	# MMU BUG ? can not do -1(a1) at 0x80000000!!
2:
	j	ra
	nop
#endif
END(memcpy)





  .text
  .align 3
  .set   noreorder


  .globl CacheFlush
  .ent   CacheFlush
  .type  CacheFlush, @function

CacheFlush:
/*++
VOID
MipsCacheSync (
  VOID
  )

Routine Description:

  Sync ALL caches.

Arguments:

    None
  
Returns:

  None
--*/

	sync

	li	t1, 64*1024
	srl	t1, t1, 2			# / 4ways
	li	t2, 64*1024
	srl	t2, t2, 2			# / 4ways

	/* L1 I$ */

	li   t0, 0x80000000
	addu t1, t0, t1			# Compute end address
	subu t1, 32
1:
	cache	0x0, 0(t0)
	bne	t0, t1, 1b
	addu t0, 32

	/* L1 D$ */

	li   t0, 0x80000000
	addu t1, t0, t2			# End address
	subu t1, 32
1:
	cache	0x01, 0(t0)
	cache	0x01, 1(t0)
	cache	0x01, 2(t0)
	cache	0x01, 3(t0)
	bne	t0, t1, 1b
	addu t0, 32

	/* L2 */

	li  t0, 0x80000000
	li	t2, 512*1024
	srl	t2, 2				# because cache is 4 way
	addu t1, t0, t2
	subu t1, 32
1:
	cache	0x03, 0(t0)
	cache	0x03, 1(t0)
	cache	0x03, 2(t0)
	cache	0x03, 3(t0)
	bne	t0, t1, 1b
	addu t0, 32

	j	ra
	nop
  
  .end CacheFlush


