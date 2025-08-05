#ifndef S21_DECIMAL_H
#define S21_DECIMAL_H

#include <stdio.h>
#define INT_MAX 2147483647
#define MAX_DECIMAL 7e28f

typedef struct {
    int bits[4];
} s21_decimal;

int s21_add(s21_decimal val1, s21_decimal val2, s21_decimal* result); //готово
int s21_sub(s21_decimal val1, s21_decimal val2, s21_decimal* result); //готово
int s21_mul(s21_decimal val1, s21_decimal val2, s21_decimal* result);
int s21_div(s21_decimal val1, s21_decimal val2, s21_decimal* result);

int s21_is_less(s21_decimal, s21_decimal); //готово
int s21_is_less_or_equal(s21_decimal, s21_decimal); //готово
int s21_is_greater(s21_decimal, s21_decimal); //готово
int s21_is_greater_or_equal(s21_decimal, s21_decimal); //готово
int s21_is_equal(s21_decimal, s21_decimal); //готово
int s21_is_not_equal(s21_decimal, s21_decimal); //готово

int s21_from_int_to_decimal(int src, s21_decimal* dst); //готово
int s21_from_float_to_decimal(float src, s21_decimal* dst);
int s21_from_decimal_to_int(s21_decimal src, int* dst); //готово
int s21_from_decimal_to_float(s21_decimal src, float* dst); //готово

int s21_floor(s21_decimal value, s21_decimal* result);
int s21_round(s21_decimal value, s21_decimal* result);
int s21_truncate(s21_decimal value, s21_decimal* result);
int s21_negate(s21_decimal value, s21_decimal* result);

int s21_get_sign(s21_decimal d); //получает знак числа
void s21_set_sign(s21_decimal* d, int sign); //устанавливает знак числа
int s21_get_scale(s21_decimal d); //получает степень числа
void s21_set_scale(s21_decimal* d, int scale); // уставливает степень
static void s21_copy_decimal(s21_decimal *dst, const s21_decimal *src); //копирует число 
static int s21_up_raw_10(s21_decimal *d); //повышет матиссу в 10 раз без изменения степени 0 норм
static void s21_down_row_10(s21_decimal *src); //понижает матиссу в 10 раз без изменения степени 0 норм
static int s21_align_scales(s21_decimal *a, s21_decimal *b); //выравнивает матиссы под одну степень 0 норм
static int s21_cmp_raw(s21_decimal a, s21_decimal b); //сравнивает матиссы без учета знака и степени -1 0 1
static int s21_add_mantiss(s21_decimal a, s21_decimal b, s21_decimal* result); //складывает матиссы
static void s21_sub_mantiss(s21_decimal maxd, s21_decimal mind, s21_decimal* result); //отнимает из большей матиссы меньшую


#endif  // S21_DECIMAL_H
