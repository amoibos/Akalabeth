#include "strings.h"

unsigned char strlen(const char * str) {
	const char *s;

	for (s = str; *s; ++s) ;

    return s - str;
}

/*FIXME: hopefully the result is never above 127*/
/*
signed char strpos(const unsigned char * search, const unsigned char * content, const unsigned char start) {
	unsigned char i, j, check;
    signed char result=-1;
	unsigned char len_search = strlen(search);
	unsigned char len_content = strlen(content);

	i = start;
	if (
        (len_search > len_content) ||
        (*search == '\0') ||
        (i > (len_content - 1))
        )
        return result;

	for(; i < len_content; ++i)	{
		check = 0;
		for(j = 0; j < len_search; ++j)
			if(content[i + j] == search[j])
				++check;

		if(check == len_search) {
			result = i;
			break;
		}

	}
	return result;
}
*/

unsigned char* SEGA_itoa(int value, char * sp) {
    unsigned int uval;
    char *start;
    char *dest = sp;
    char t;

    if (value < 0) {
        *sp++ = '-';
        uval = (unsigned int)(-value);
    } else {
        uval = (unsigned int)value;
    }

    start = sp;
    do {
        *sp++ = '0' + (unsigned char)(uval % 10U);
        uval /= 10U;
    } while (uval);

    *sp-- = '\0';
    while (start < sp) {
        t = *start;
        *start++ = *sp;
        *sp-- = t;
    }
    return dest;
}

_Bool is_number(const char * str) {

    do {
         if ((*str < '0') || (*str > '9'))
             return 0;
    } while (*(++str));
    return 1;
}

int SEGA_atoi(const char * str) {
	int k=0;
    unsigned char start;

    start = *str;
	while (*str)
	{
		k = (k << 3) + (k << 1) + (*str) - '0';
		++str;
	}
	return k * ((start == '-') ? -1 : 1);
}

char* strcat(char * dest, char * src) {
	char *rdest=dest;

	while (*dest)	dest++;
	while (*dest++ = *src++) ;

	return rdest;
}

/*
signed char strcmp(const char * str1, const char * str2) {
	while(*str1 && (*str1 == *str2)) {
		++str1;
		++str2;
	}
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
} */
/*
void strncpy(unsigned char* dst, const unsigned char* src, const unsigned char amount) {
	unsigned char i = 0;
	while(i++ != amount && (*dst++ = *src++));
}*/

char* strcpy(char * dest, const char* src) {
	char * rdest=dest;

    while(*dest++ = *src++);

    return rdest;
}

unsigned char upcase(const unsigned char c) {

   if ((c >= 'a') && (c <='z'))
        return c - 32;
    else
        return c;

}
/*
signed char strchr(const char *str, const char ch) {
	unsigned char result=0;

    for (; str[result] != '\0'; ++result)
  		if (str[result] == ch)
    		return result;

	return -1;
} */

/*
unsigned char is_alpha(const unsigned char mark) {
	return 	(mark >= 0x41) && (mark <= 0x5A) ||
			(mark >= 0x61) && (mark <= 0x7A);
}*/

/*long long pow(long long base, int ex) {
	long long result = base;
	for(int i = 0; i < ex; ++i)
		result *= base;
	return result;
} */

/*
char * SEGA_ftoa(float value, int decimals, char * buf) {
    unsigned int index = 0;
    unsigned int d;

    // Handle negative values
    if (value < 0) {
        buf[index] = '-';
        index++;
        value = -value;
    }

    // Rounding
    float rounding = 0.5;
    for (d = 0; d < decimals; rounding /= 10.0, d++);
    value += rounding;

    // Integer part - ohne Rekursion
    int int_part = (int)value;
    int temp = int_part;
    int digits = 0;

    // Anzahl der Ziffern bestimmen
    if (temp == 0) {
        digits = 1;
    } else {
        while (temp > 0) {
            temp /= 10;
            digits++;
        }
    }

    // Ziffern von rechts nach links berechnen und speichern
    int start_pos = index;
    for (int i = digits - 1; i >= 0; i--) {
        buf[start_pos + i] = (int_part % 10) + 0x30;
        int_part /= 10;
    }
    index += digits;

    if (decimals)
        buf[index++] = '.';

    // Decimal part
    value = value - (int)value;
    int ival = 1;
    for (d = 0; d < decimals; ival *= 10, d++);
    ival = (int)(ival * value);

    // Dezimalstellen ohne Rekursion
    temp = ival;
    digits = decimals; // Dezimalstellen haben feste Länge

    // Ziffern von rechts nach links
    start_pos = index;
    for (int i = digits - 1; i >= 0; i--) {
        buf[start_pos + i] = (temp % 10) + 0x30;
        temp /= 10;
    }
    index += digits;

    buf[index] = '\0';
    return buf;
}
*/

// WARNING: capitalize on src string!
char * to_upper(char * src) {
	char *temp;

	for (temp = src; *temp; ++temp)
		*temp = (char)upcase(*temp);
	return src;
}
/*
unsigned char is_digit(const unsigned char ch) {
	return (ch >= '0') && (ch <= '9');
}*/
/*
unsigned char * sprint(unsigned char * buffer, unsigned char * format, unsigned char **strings, long  * numbersInt, float * numbersFloat) {
    unsigned char * str;
    long integer;
    float real;
    unsigned char buffer_pos=0;
    unsigned char temp[80+1];
    char * start;

    start = buffer;
    buffer[0] = 0;
    for (char format_pos=0; format_pos < strlen(format); ++format_pos) {
        if (format[format_pos] != '%') {
            buffer[buffer_pos++] = format[format_pos];
        } else if ((format[format_pos] == '%') && (format[format_pos+1] == '%')) {
            buffer[buffer_pos++] = format[format_pos++];
        } else {
            ++format_pos;
            switch (format[format_pos]) {
                case 's': {
                    str = *strings;
                    ++strings;
                    buffer[buffer_pos] = 0;
                    strcat(buffer, str);
                    break;
                }
                case 'd': {
                    integer = *numbersInt++;
                    buffer[buffer_pos] = 0;
                    SEGA_itoa(integer, temp);
                    strcat(buffer, temp);
                    break;
                }
                  case 'f': {
                    real = *numbersFloat++;
                    buffer[buffer_pos] = 0;
                    ftoa(real, 10, temp);
                    strcat(buffer, temp);
                    break;
                }
                default: {
                    break;
                }
            }

        }
    }
    buffer[buffer_pos] = 0;
    return start;
}
*/
