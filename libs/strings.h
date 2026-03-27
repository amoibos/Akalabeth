#ifndef STRINGS_H
#define STRINGS_H

char* SEGA_itoa(int value, char * sp);
int SEGA_atoi(const char * str);
//char* SEGA_ftoa(float value, int decimals, char* buf);


char* strcat(char * dest, char * src);

//signed char strcmp(unsigned char * str1, const unsigned char * str2);
//void strncpy(unsigned char * dest, const unsigned char * src, const unsigned char amount);
unsigned char strlen(const char * str);
//signed char strpos(unsigned char* search, unsigned char * content, unsigned char start);
char* strcpy(char * dest, const char * src);

//signed char strchr(const char *str, char ch);
_Bool is_number(const char * str);
unsigned char upcase(const unsigned char c);
//unsigned char is_alpha(const unsigned char mark);
char * to_upper(char * src);
//unsigned char is_digit(const unsigned char ch);
//unsigned char * sprint(unsigned char * buffer, const unsigned char * format, const unsigned char **strings, const long * numbersInt, const float * numbersFloat);


#endif
