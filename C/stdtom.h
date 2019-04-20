/*
 * Copyright (c) 2019 Tom. All Rights Reserved.
 *
 * @TOM_LICENSE_HEADER_START@
 *
 * 1) Credit would be sick, but I really can't control what you do ¯\_(ツ)_/¯
 * 2) I'm not responsible for what you do with this AND I'm not responsible for any damage you cause ("THIS SOFTWARE IS PROVIDED AS IS", etc)
 * 3) I'm under no obligation to provide support. (But if you reach out I'll gladly take a look if I have time)
 *
 * @TOM_LICENSE_HEADER_END@
 */
/*
 * A collection of useful functions, particularly with obscure usecases
 */

#ifndef stdtom_h
#define stdtom_h

#include <stdio.h>
#include <stdbool.h>




#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

// just in case a more explicit name is desired
//#define log(format, ...) lprintf(format, ##__VA_ARGS__) // uncomment at your own risk, may cause conflicts with log math function

// TODO
//// alt name for dgblprintf
//#define dbglog(format, ...) dbglprintf(format, ##__VA_ARGS__)
//
//// for debugging with normal printf
//#define dbgprintf(format, ...) printf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#pragma clang diagnostic pop


// do nothing - primarily for use to get rid of the 'variable not used' warning when warnings are treated as errors
#define NOTUSED(x) (void)(x)



// unsigned
#define BYTE   unsigned char           // 1 Byte
#define WORD   unsigned short int      // 2 Bytes
#define DWORD  unsigned int            // 4 Bytes
#define QWORD  unsigned long long int  // 8 Bytes





// selects bits `from` through `to` from `source`.
short select_bit_range_from_word(WORD source, int from, int through);


// converts hex string to an integer
int hextodec(char* hex_string);

// converts a string to an integer - unlike atoi & friends, it tells you if and why it fails
int strtonum(char *numstr, int minval, int maxval, char **errstrp);


// creates a string from a format
char* string_with_format(char* format, ...);

// appends s2 to s1 and returns the result
char* strapp(const char *s1, const char *s2);

// appends char to string (one small caveat:  if the string isn't big enough this will break)
void str_append_char(char* str, char character);

// trims strings down to their real size. Also removes newline characters.
char* strtrim (char* str);

// zeros out string - apprently there's a thing called bzero also does this?
void zero_str(char** str);

// checks if a string contains all 0's (as it won't be equal to NULL)
bool str_is_0d(char* str);


// reads the contents of a file into a string
char* str_from_file_contents(char* filename);

// returns path extension of path
char* path_extension(char* path);

// checks if a path is a directory
bool isdir(char* path);

// gets the file size for a file
size_t filesize(char* file_path);


#ifdef __APPLE__
/*
 	gets the path to the executable belonging the the handle given by dlopen()
 	Warning: this is not a thread safe function
 */
const char* path_from_handle(void* handle);


// retruns the path to the executable that calls it
const char* path_to_current_executable(void);
#endif


// getopt - but actually resonable to use
// no use overloading argv[], its basically always a c string
char* getarg(int argc, char* argv[], char* argname);

// checks if arg is present
bool arg_is_present(int argc, char* argv[], char* argname);


// set the prefix used by lprintf
void set_lprefix(char* new_prefix);

// printf, but with a prefix and ending newling - 'l' for 'log'
void lprintf(const char* format, ...);

// clears the console
void printclr(void);


#endif /* stdtom_h */
