#include "m_theme.h"

int red_gradient(int iter,int accuracy){
    int col = 0;
    if(iter<accuracy){
        double t = (double)iter/(double)accuracy;
        col |= 255<<16;//Channel red
        col |= (int)(255.0- 255.0 * t)<<8;//Channel green
        col |=(int)(255.0- 255.0 * t);//Channel blue
    }
    return col;
}
int r_blue_gradient(int iter,int accuracy){
    int col = 0;
    if(iter<accuracy){
        double t = (double)iter/(double)accuracy;
        col |= 0<<16;//Channel red
        col |= 0<<8;//Channel green
        col |= (int)(255.0 * t);//Channel blue
    }
    return col;
}
int blue_gradient(int iter,int accuracy){
    int col = 0;
    if(iter<accuracy){
        double t = (double)iter/(double)accuracy;
        col |= (int)(255.0- 255.0 * t)<<16;//Channel red
        col |= (int)(255.0- 255.0 * t)<<8;//Channel green
        col |= 255;//Channel blue
    }
    return col;
}
M_Theme_Prim* m_theme_get(M_Theme theme_id){
    M_Theme_Prim* theme = malloc(sizeof(M_Theme_Prim));
    switch (theme_id)
    {
    case RED_GRADIENT:
        theme->theme_name = "RED GRADIENT";
        theme->fptr = &red_gradient;
        break;
    case BLUE_GRADIENT:
        theme->theme_name = "BLUE GRADIENT";
        theme->fptr = &blue_gradient;
        break;
    case R_BLUE_GRADIENT:
        theme->theme_name = "REVERSED BLUE GRADIENT";
        theme->fptr = &r_blue_gradient;
        break; 
    default:
        break;
    }
    return theme;
}
