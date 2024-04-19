#ifndef UTIlS_H
#define UTILS_H

int wrap(int x, int len) {
    if(len == 0) return 0;
    while(x<0) x+= len;
    while(x>=len) x-= len;
    return x;
}

#define FPMUL(x, y, b) (((x)*(y))>>(b))

#endif UTILS_H