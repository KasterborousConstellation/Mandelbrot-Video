#ifndef MTHEME
#define MTHEME
#include <stdlib.h>
#define Color char[3] 
typedef enum M_Theme {
    RED_GRADIENT,
    BLUE_GRADIENT,
    R_BLUE_GRADIENT
} M_Theme;
typedef struct M_Theme_Prim {char* theme_name;int (*fptr)(int,int);} M_Theme_Prim;
M_Theme_Prim* m_theme_get(M_Theme theme_id);
#endif