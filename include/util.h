#ifndef _UTIL_H_
#define _UTIL_H_

//Generovani nahodneho cisla typu float
static float frand() { return rand() / (float) RAND_MAX; }
static float frand(float f) { return frand() * f; }
static float frand(float f, float g) { return frand(g - f) + f; }

//Vypocet prepony v pravouhlem trojuhelniku
static float pythagoras_c(float a, float b) { return sqrt(a*a+b*b); }

#endif
