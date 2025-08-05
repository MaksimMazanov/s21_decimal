#include "s21_decimal.h"

#define S21_SIGN_MASK    0x80000000
#define S21_SCALE_MASK   0x00FF0000

int s21_get_sign(s21_decimal d) {
    return (d.bits[3] & S21_SIGN_MASK) ? 1 : 0;
}

void s21_set_sign(s21_decimal* d, int sign) {
    if (sign)
        d->bits[3] |= S21_SIGN_MASK;
    else
        d->bits[3] &= ~S21_SIGN_MASK;
}

int s21_get_scale(s21_decimal d) {
    return (d.bits[3] >> 16) & 0xFF;
}

void s21_set_scale(s21_decimal* d, int scale) {
    d->bits[3] &= ~S21_SCALE_MASK;
    d->bits[3] |= ((scale & 0xFF) << 16);
}

static void s21_copy_decimal(s21_decimal *dst, const s21_decimal *src) {
    for (int i = 0; i < 4; ++i) dst->bits[i] = src->bits[i];
}

static int s21_up_raw_10(s21_decimal *d) {
    unsigned long long temp = 0;
    unsigned long long carry = 0;
    for (int i = 0; i < 3; ++i) {
        temp = ((unsigned long long)d->bits[i]) * 10 + carry;
        d->bits[i] = (unsigned int)(temp & 0xFFFFFFFF);
        carry = temp >> 32;
    }
    if (carry != 0) return 1;
    return 0;
}

static int s21_align_scales(s21_decimal *a, s21_decimal *b) {
    int scale_a = s21_get_scale(*a), scale_b = s21_get_scale(*b);
    int diff = scale_a - scale_b;

    if (diff) {
        s21_decimal *to_scale = diff < 0 ? a : b;
        int steps = diff < 0 ? -diff : diff;

        while (steps-- > 0) {
            if (s21_up_raw_10(to_scale)) return 1;
        }

        int new_scale = diff < 0 ? scale_b : scale_a;
        s21_set_scale(a, new_scale);
        s21_set_scale(b, new_scale);
    }
    return 0;
}

static int s21_cmp_raw(s21_decimal a, s21_decimal b) {
    
    for (int i = 2; i >= 0; --i) {
        if (a.bits[i] != b.bits[i]) {
            return (a.bits[i] < b.bits[i]) ? -1 : 1;
        }
    }
    return 0; 
}

int s21_is_less(s21_decimal a, s21_decimal b) {
    int result = 0;
    int sign_a = s21_get_sign(a), sign_b = s21_get_sign(b);

    if (sign_a != sign_b) {
        result = sign_a && !sign_b;
    } else {
        s21_decimal val1 = a, val2 = b;
        if (!s21_align_scales(&val1, &val2)) {
            int r = cmp_raw(val1, val2);
            if (r != 0) {
                result = (sign_a) ? (r > 0) : (r < 0);
            }
        }
    }
    return result;
}

int s21_is_equal(s21_decimal a, s21_decimal b) {
    int result = 0;
    s21_decimal val1 = a, val2 = b;

    if (!s21_align_scales(&val1, &val2) && cmp_raw(val1, val2) == 0) {
        unsigned int mant = val1.bits[0] | val1.bits[1] | val1.bits[2];
        if (!mant) {
            result = 1;
        } else {
            result = (s21_get_sign(val1) == s21_get_sign(val2));
        }
    }
    return result;
}

int s21_is_less_or_equal(s21_decimal a, s21_decimal b) {
    return s21_is_less(a, b) || s21_is_equal(a, b);
}

int s21_is_greater(s21_decimal a, s21_decimal b) {
    return s21_is_less(b, a);
}

int s21_is_greater_or_equal(s21_decimal a, s21_decimal b) {
    return s21_is_greater(a, b) || s21_is_equal(a, b);
}

int s21_is_not_equal(s21_decimal a, s21_decimal b) {
    return !s21_is_equal(a, b);
}

static int s21_add_mantiss(s21_decimal a, s21_decimal b, s21_decimal* result) {
    unsigned long long buffer = 0;

    for (int i = 0; i < 3; ++i) {
        buffer = (unsigned long long)a.bits[i] + (unsigned long long)b.bits[i] + (buffer >> 32);
        result->bits[i] = (unsigned int)(buffer & 0xFFFFFFFF);
    }
    return (buffer >> 32) ? 1 : 0;  
}

static void s21_sub_mantiss(s21_decimal maxd, s21_decimal mind, s21_decimal* result) {
    long long borrow = 0;
    for (int i = 0; i < 3; ++i) {
        long long diff = (long long)maxd.bits[i] - (long long)mind.bits[i] - borrow;
        if (diff < 0) {
            diff += ((long long)1 << 32);
            borrow = 1;
        } else {
            borrow = 0;
        }
        result->bits[i] = (unsigned int)diff;
    }
}

int s21_add(s21_decimal a, s21_decimal b, s21_decimal *result) {
    int error_code = 1; 
    if (result) {
        for (int i = 0; i < 4; ++i) result->bits[i] = 0;

        s21_decimal val1 = a, val2 = b;

        if (!s21_align_scales(&val1, &val2)) {
            int scale = s21_get_scale(val1);
            int sign1 = s21_get_sign(val1), sign2 = s21_get_sign(val2);
            int res_sign = 0;

            if (sign1 == sign2) {
                res_sign = sign1;
                error_code = s21_add_mantiss(val1, val2, result);
            } else {
                int cmp = s21_cmp_raw(val1, val2);
                s21_decimal maxd = (cmp >= 0) ? val1 : val2;
                s21_decimal mind = (cmp >= 0) ? val2 : val1;
                res_sign = (cmp >= 0) ? sign1 : sign2;

                s21_sub_mantiss(maxd, mind, result);
                error_code = 0;
            }

            s21_set_scale(result, scale);
            s21_set_sign(result, res_sign);
        }
    }
    return error_code;
}

int s21_sub(s21_decimal a, s21_decimal b, s21_decimal *result) {
    int error_code = 1; 
    if (result) {
       
        s21_decimal neg_b = b;
        int sign_b = s21_get_sign(neg_b);
        s21_set_sign(&neg_b, !sign_b);
        
        error_code = s21_add(a, neg_b, result);
    }
    return error_code;
}

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
    int error = 1;
    if (dst) {
        memset(dst, 0, sizeof(s21_decimal));
        if (src < 0) {
            s21_set_sign(dst, 1);
            src = -src;
        }
        dst->bits[0] = (unsigned int)src;
        s21_set_scale(dst, 0);
        error = 0;
    }
    return error;
}

static void s21_down_raw_10(s21_decimal *src) {
    unsigned long long rem = 0;
    for (int i = 2; i >= 0; --i) {
        unsigned long long cur = (rem << 32) | src->bits[i];
        src->bits[i] = (unsigned int)(cur / 10ull);
        rem = cur % 10ull;
    }
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
    if (dst) {
        int sign = s21_get_sign(src);
        int scale = s21_get_scale(src);

        while (scale-- > 0) {
            s21_down_raw_10(&src);
        }

        if (src.bits[2] != 0 || src.bits[1] != 0 || src.bits[0] > (unsigned)INT_MAX) {
            *dst = 0;
            return 1; 
        }

    *dst = sign ? -(int)src.bits[0] : (int)src.bits[0];
    return 0;  // OK
    }
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
    int error = 1;
    if (dst) {
        unsigned long long mant = 0;
        mant |= (unsigned long long)src.bits[2] << 64; 
        mant |= (unsigned long long)src.bits[1] << 32;
        mant |= src.bits[0];
        int scale = s21_get_scale(src);
        double tmp = (double)mant;
        for (int i = 0; i < scale; ++i) tmp /= 10.0;
        if (s21_get_sign(src)) tmp = -tmp;
        *dst = (float)tmp;
        if (isfinite(*dst)) {
            error = 0;
        } else {
            *dst = 0.0f;
        }
    }
    return error;
}