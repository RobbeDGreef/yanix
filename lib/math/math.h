#ifndef MATH_H
#define MATH_H

#define PI 3.14159265359

double factorial(int n);
int flip(int n);
int abs(int n);
float absf(float n);
double sqrt(double square);
int pow(int x, int n);
int ceil(float num);
void swap(int*a , int*b);

/**
 * @brief      rounds up number if it's not zero
 *
 * @param[in]  x     the number to round up
 *
 * @return     the rounded number
 */
int roundup(int x, int y);

#endif