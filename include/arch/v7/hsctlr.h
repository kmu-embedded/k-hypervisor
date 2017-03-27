#ifndef __HSCTLR_H__
#define __HSCTLR_H__

#define HSCTLR_TE                   30
#define HSCTLR_EE                   25
#define HSCTLR_FI                   21
#define HSCTLR_WXN                  19
#define HSCTLR_I                    12
#define HSCTLR_CP15BEN              5
#define HSCTLR_C                    2 
#define HSCTLR_A                    1 
#define HSCTLR_M                    0


#define TE_BIT                      (0b1)
#define EE_BIT                      (0b1)
#define FI_BIT                      (0b1)
#define WXN_BIT                     (0b1)
#define I_BIT                       (0b1)
#define CP15BEN_BIT                 (0b1)
#define C_BIT                       (0b1)
#define A_BIT                       (0b1) 
#define M_BIT                       (0b1)

#define HSCTLR_BIT(x)                      ( x##_BIT << HSCTLR_##x )

#endif
