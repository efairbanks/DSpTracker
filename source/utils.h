#ifndef UTIlS_H
#define UTILS_H

int wrap(int x, int len) {
    if(len == 0) return 0;
    while(x<0) x+= len;
    while(x>=len) x-= len;
    return x;
}

int clip(int x, int min, int max) {
    if(x < min) return min;
    if(x > max) return max;
    return x;
}

int clip16(int x) {
    return clip(x, -32768, 32767);
}

#define FPMUL(x, y, b) (((x)*(y))>>(b))

#endif UTILS_H