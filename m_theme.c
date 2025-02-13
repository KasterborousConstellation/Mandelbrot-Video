#include "m_theme.h"
int getRGB(int red, int green, int blue){
    return ((red&255)<<16)|((green&255)<<8)|(blue&255);
}
int reds[] = {66,25,9,4,0,12,24,57,134,211,241,248,255,204,153,106};
int greens[] = {30,7,1,4,7,44,82,125,181,236,233,201,170,128,87,52};
int blues[] = {15,26,47,73,100,138,177,209,229,248,191,95,0,0,0,3};
int colorful_gradient(int iter, int accuracy){
    int i = iter%16;
    return (iter>=accuracy)?0:getRGB(reds[i],greens[i],blues[i]);
}
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
    case COLORFUL_GRADIENT:
        theme->theme_name = "COLORFUL GRADIENT";
        theme->fptr = &colorful_gradient;
        break;
    default:
        break;
    }
    return theme;
}
