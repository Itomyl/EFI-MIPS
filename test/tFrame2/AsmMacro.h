#ifndef _ASM_MACRO_H_
#define _ASM_MACRO_H_

#define _KERNEL

#ifdef NEED_OLD_RM7KFIX
#define ITLBNOPFIX      nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
#else
#define ITLBNOPFIX      nop;nop;nop;nop
#endif

#define _MIPS_ISA_MIPS1 1 /* R2000/R3000 */
#define _MIPS_ISA_MIPS2 2 /* R4000/R6000 */
#define _MIPS_ISA_MIPS3 3 /* R4000 */
#define _MIPS_ISA_MIPS4 4 /* TFP (R1x000) */

#if !defined(ABICALLS) && !defined(_NO_ABICALLS)
#define ABICALLS  .abicalls
#endif

#if defined(ABICALLS) && !defined(_KERNEL)
  ABICALLS
#endif

#define _C_LABEL(x) x   /* XXX Obsolete but keep for a while */

#if !defined(__MIPSEL__) && !defined(__MIPSEB__)
#error "__MIPSEL__ or __MIPSEB__ must be defined"
#endif
  /*
   *  * Define how to access unaligned data word
   *   */
#if defined(__MIPSEL__)
#define LWLO    lwl
#define LWHI    lwr
#define SWLO  swl
#define SWHI  swr
#define LDLO    ldl
#define LDHI    ldr
#define SDLO  sdl
#define SDHI  sdr
#endif
#if defined(__MIPSEB__)
#define LWLO    lwr
#define LWHI    lwl
#define SWLO  swr
#define SWHI  swl
#define LDLO    ldr
#define LDHI    ldl
#define SDLO  sdr
#define SDHI  sdl
#endif

  /*
   *  *  Define programming environment for ABI.
   *   */
#if defined(ABICALLS) && !defined(_KERNEL) && !defined(_STANDALONE)
#warning xxxxxxxxxxxxxxx

#ifndef _MIPS_SIM
#define _MIPS_SIM 1
#define _ABIO32 1
#endif
#ifndef _MIPS_ISA
#define _MIPS_ISA 2
#define _MIPS_ISA_MIPS2 2
#endif

#if (_MIPS_SIM == _ABIO32) || (_MIPS_SIM == _ABI32)
#define NARGSAVE  4

#define SETUP_GP    \
      .set  noreorder;  \
    .cpload t9;   \
    .set  reorder;

#define SAVE_GP(x)    \
      .cprestore x

#define SETUP_GP64(gpoff, name)
#define RESTORE_GP64
#endif

#if (_MIPS_SIM == _ABI64) || (_MIPS_SIM == _ABIN32)
#define NARGSAVE  0

#define SETUP_GP
#define SAVE_GP(x)
#define SETUP_GP64(gpoff, name) \
      .cpsetup t9, gpoff, name
#define RESTORE_GP64    \
      .cpreturn
#endif

#define MKFSIZ(narg,locals) (((narg+locals)*REGSZ+31)&(~31))

#else /* defined(ABICALLS) && !defined(_KERNEL) */

#define NARGSAVE  4
#define SETUP_GP
#define SAVE_GP(x)

#define ALIGNSZ   16  /* Stack layout alignment */
#define FRAMESZ(sz) (((sz) + (ALIGNSZ-1)) & ~(ALIGNSZ-1))

#endif

  /*
   *  *  Basic register operations based on selected ISA
   *   */
#if (_MIPS_ISA == _MIPS_ISA_MIPS1 || _MIPS_ISA == _MIPS_ISA_MIPS2)
#warning  44444444444
#define REGSZ   4 /* 32 bit mode register size */
#define LOGREGSZ  2 /* log rsize */
#define REG_S sw
#define REG_L lw
#define CF_SZ   24  /* Call frame size */
#define CF_ARGSZ  16  /* Call frame arg size */
#define CF_RA_OFFS  20  /* Call ra save offset */
#endif

#if (_MIPS_ISA == _MIPS_ISA_MIPS3 || _MIPS_ISA == _MIPS_ISA_MIPS4)
#warning 888888888888888888888
#define REGSZ   8 /* 64 bit mode register size */
#define LOGREGSZ  3 /* log rsize */
#define REG_S sd
#define REG_L ld
#define CF_SZ   48  /* Call frame size (multiple of ALIGNSZ) */
#define CF_ARGSZ  32  /* Call frame arg size */
#define CF_RA_OFFS  40  /* Call ra save offset */
#endif

#ifndef __LP64__
#warning no __LP64__
#define PTR_L   lw
#define PTR_S   sw
#define PTR_SUB   sub
#define PTR_ADD   add
#define PTR_SUBU  subu
#define PTR_ADDU  addu
#define LI    li
#define LA    la
#define PTR_SLL   sll
#define PTR_SRL   srl
#define PTR_VAL   .word
#else
#warning __LP64__
#define PTR_L   ld
#define PTR_S   sd
#define PTR_ADD   dadd
#define PTR_SUB   dsub
#define PTR_SUBU  dsubu
#define PTR_ADDU  daddu
#define LI    dli
#define LA    dla
#define PTR_SLL   dsll
#define PTR_SRL   dsrl
#define PTR_VAL   .dword
#endif

  /*
   *  * Define -pg profile entry code.
   *   */
#if defined(XGPROF) || defined(XPROF)
#define MCOUNT      \
      PTR_SUBU sp, sp, 64;  \
    SAVE_GP(16);    \
    sd  ra, 56(sp); \
    sd  gp, 48(sp); \
    .set  noat;   \
    .set  noreorder;  \
    move  AT, ra;   \
    jal _mcount;  \
    PTR_SUBU sp, sp, 16;  \
    ld  ra, 56(sp); \
    PTR_ADDU sp, sp, 64;  \
    .set reorder;   \
    .set  at;
#else
#define MCOUNT
#endif

  /*
   *  * LEAF(x, fsize)
   *   *
   *    * Declare a leaf routine.
   *     */
#define LEAF(x, fsize)    \
      .align  3;    \
    .globl x;   \
    .ent x, 0;    \
  x: ;        \
    .frame sp, fsize, ra; \
    SETUP_GP    \
    MCOUNT

#define ALEAF(x)    \
      .globl  x;    \
  x:

  /*
   *  * NLEAF(x)
   *   *
   *    * Declare a non-profiled leaf routine.
   *     */
#define NLEAF(x, fsize)   \
      .align  3;    \
    .globl x;   \
    .ent x, 0;    \
  x: ;        \
    .frame sp, fsize, ra; \
    SETUP_GP

  /*
   *  * NON_LEAF(x)
   *   *
   *    * Declare a non-leaf routine (a routine that makes other C calls).
   *     */
#define NON_LEAF(x, fsize, retpc) \
      .align  3;    \
    .globl x;   \
    .ent x, 0;    \
  x: ;        \
    .frame sp, fsize, retpc; \
    SETUP_GP    \
    MCOUNT

  /*
   *  * NNON_LEAF(x)
   *   *
   *    * Declare a non-profiled non-leaf routine
   *     *  (a routine that makes other C calls).
   *      */
#define NNON_LEAF(x, fsize, retpc) \
      .align  3;    \
    .globl x;   \
    .ent x, 0;    \
  x: ;        \
    .frame sp, fsize, retpc \
    SETUP_GP

  /*
   *  * END(x)
   *   *
   *    * Mark end of a procedure.
   *     */
#define END(x) \
      .end x

  /*
   *  * Macros to panic and printf from assembly language.
   *   */
#define PANIC(msg) \
      LA  a0, 9f; \
    jal panic;  \
    nop ; \
    MSG(msg)

#define PRINTF(msg) \
      LA  a0, 9f; \
    jal printf; \
    nop ; \
    MSG(msg)

#define MSG(msg) \
      .rdata; \
  9:  .asciiz msg; \
    .text

#define ASMSTR(str) \
      .asciiz str; \
    .align  3

#define LOAD_XKPHYS(reg, cca) \
      li  reg, cca | 0x10; \
    dsll  reg, reg, 59

#ifdef MULTIPROCESSOR
#define GET_CPU_INFO(ci, tmp)   \
      LOAD_XKPHYS(ci, CCA_CACHED);  \
    mfc0  tmp, COP_0_LLADDR;  \
    or  ci, ci, tmp
#else  /* MULTIPROCESSOR */
#define GET_CPU_INFO(ci, tmp)   \
      LA  ci, cpu_info_primary
#endif /* MULTIPROCESSOR */







#endif /* _ASM_MACRO_H_ */
