/*
 * Copyright (c) 2019 Tom. All Rights Reserved.
 *
 * @TOM_LICENSE_SOURCE_START@
 *
 * 1) Credit would be sick, but I really can't control what you do ¯\_(ツ)_/¯
 * 2) I'm not responsible for what you do with this AND I'm not responsible for any damage you cause ("THIS SOFTWARE IS PROVIDED AS IS", etc)
 * 3) I'm under no obligation to provide support. (But if you reach out I'll gladly take a look if I have time)
 *
 * @TOM_LICENSE_SOURCE_END@
 */
/*
 * A collection of useful functions, particularly with obscure usecases
 */

#include "stdtom.h"

#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>




#ifndef __APPLE__ // For consistent behavior across platforms
#define ARG_MAX            (256 * 1024)
#endif


#define	INVALID		1
#define	TOOSMALL	2
#define	TOOLARGE	3





char* lprefix = "";
NSString *logPrefix = @"";




// inspired by answers to this stack overflow question: https://stackoverflow.com/questions/10090326/how-to-extract-specific-bits-from-a-number-in-c
short select_bit_range_from_word(WORD source, int from, int through)
{
	short selection;
	
	
	through++; // the function does things "to", not "through"
	
	
	// average of the top two answers
	selection = (source >> from) & ((1 << (through - from)) - 1);
	
	
	return selection;
}




int hexToDecimal(NSString *hexString)
{
	int hex;
	sscanf([hexString UTF8String], "%x", &hex);
	
	
	return hex;
}




int hextodec(char* hex_string)
{
	int hex;
	sscanf(hex_string, "%x", &hex);
	
	
	return hex;
}




// 100% stolen from the FreeBSD source code: https://github.com/freebsd/freebsd/blob/master/lib/libc/stdlib/strtonum.c
// Modified to use ints because WHO actually uses long longs - so really like 95% stolen from BSD
// Also changed to normal char - const chars are really just obnoxious and not strictly needed here
int strtonum(char *numstr, int minval, int maxval, char **errstrp)
{
	int num = 0;
	int error = 0;
	char *ep;
	struct errval {
		char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};
	
	ev[0].err = errno;
	errno = 0;
	if (minval > maxval) {
		error = INVALID;
	} else {
		num = (int)strtol(numstr, &ep, 10);
		if (errno == EINVAL || numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((num == INT_MIN && errno == ERANGE) || num < minval)
			error = TOOSMALL;
		else if ((num == INT_MAX && errno == ERANGE) || num > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr; // we may also want to change this to an strdup...
	errno = ev[error].err;
	if (error)
		num = 0;
	
	return (num);
}




char* string_with_format(char* format, ...)
{
	char* formatted;
	
	char* raw_format;
	char raw_formatted[ARG_MAX]; // resuing ARG_MAX because it's the largest string we can expect
	
	
	// copy the string to a c string
	raw_format = (char* )malloc((sizeof(char) * strlen(format)) + 1);
	
	strcpy(raw_format, format);
	
	
	// Actual processing
	va_list args;
	va_start(args, format);
	
	vsnprintf(raw_formatted, ARG_MAX, raw_format, args);
	
	va_end(args);
	
	
	formatted = strdup(raw_formatted);
	
	
	return formatted;
}




// stolen from https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c/8465083
char* strapp(const char *s1, const char *s2) //strapp = string append
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    // TODO: Check malloc errors
    strcpy(result, s1);
    strcat(result, s2);
    
    
    return result;
}




// One small caveat - if the string isn't big enough this will break
void str_append_char(char* str, char character)
{
	long length = strlen(str);
	str[length] = character;
	str[(length + 1)] = '\0';
}




// trims strings down to their real size. Also removes newline characters.
char* strtrim (char* str)
{
    char* trimmed = malloc((sizeof(char) * strlen(str)));
    memcpy(trimmed, str, strlen(str));
//
//
//    //we want to trim newlines too - if they are present
//    if (trimmed[(strlen(trimmed) - 1)] == '\n')
//    {
//        trimmed[(strlen(trimmed) - 1)] = '\0';
//
//        char* newtered = malloc((sizeof(char) * (strlen(trimmed)))); // no need to subtract one now, because strlen will stop at the new '\0'
//        memcpy(newtered, trimmed, (strlen(trimmed)));
//
//
//        return strdup(newtered); //strdup may not be what we want?
//    }
//    else
//    {
		return strdup(trimmed); //strdup may not be what we want?
//    }
}




// zeros out string
void zero_str(char** str)
{
	memset(*str, 0, strlen(*str));
}




// checks if string is all 0'd out
//TODO make it check for pattern
bool str_is_0d(char* str)
{
	if (strlen(str) > 0)
	{
		for (unsigned long i = 0; i < strlen(str); i++)
		{
			if (str[i] != 0)
			{
				return false;
			}
		}
		
		
		return true;  // we could be more succinct and just let everything drop out to true, but I prefer to be explicit
	}
	else
	{
		// sure, we'll call zero-length zero'd
		return true;
	}
}




// Not NSString as it already has a function like this
char* str_from_file_contents(char* filename)
{
	char* contents = NULL;
	
	
	FILE* file_handle = fopen(filename, "r");
	
	if (file_handle != NULL)
	{
		fseek(file_handle, 0L, SEEK_END);
		long length = ftell(file_handle);
		
		rewind(file_handle);
		
		contents = malloc(length);
		
		if (contents != NULL)
		{
			fread(contents, length, 1, file_handle);
			fclose(file_handle);
		}
	}
	
	
	// cleanup
	if (file_handle != NULL)
	{
		fclose(file_handle);
	}
	
	
	contents[strlen(contents)] = '\0';
	
	
	return contents;
}



char* path_extension(char* path)
{
	const char *dot = strrchr(path, '.');
	if(!dot || dot == path) return "";
	
	
	return strdup(dot + 1);
}




bool isdir(char* path)
{
	struct stat myFile;
	
	
	if (stat(path, &myFile) < 0)
	{
		return false;
	}
	else if (S_ISDIR(myFile.st_mode))
	{
		return true;
	}
	else
	{
		return false;
	}
}




// stolen from: https://techoverflow.net/2013/08/21/a-simple-mmap-readonly-example/
size_t fileSize(NSString *filePath)
{
	struct stat st;
	
	
	stat([filePath UTF8String], &st);
	
	
	return st.st_size;
}




size_t filesize(char* file_path)
{
	struct stat st;
	
	
	stat(file_path, &st);
	
	
	return st.st_size;
}




// Based on https://www.boost.org/doc/libs/1_63_0/boost/dll/detail/posix/path_from_handle.hpp
// NOT a thread safe way of doing things
NSString *pathFromHandle(void *handle)
{
	// Since we know the image we want will always be near the end of the list, start there and go backwards
	for (uint32_t i = (_dyld_image_count() - 1); i >= 0; i--)
	{
		const char* image_name = _dyld_get_image_name(i);
		
		// Why dlopen doesn't effect _dyld stuff: if an image is already loaded, it returns the existing handle.
		void* probe_handle = dlopen(image_name, RTLD_LAZY);
		dlclose(probe_handle);
		
		if (handle == probe_handle)
		{
			return [NSString stringWithUTF8String: image_name];
		}
	}
	
	
	return NULL;
}




char* path_from_handle(void* handle)
{
	// Since we know the image we want will always be near the end of the list, start there and go backwards
	for (uint32_t i = (_dyld_image_count() - 1); i >= 0; i--)
	{
		const char* image_name = _dyld_get_image_name(i);
		
		// Why dlopen doesn't effect _dyld stuff: if an image is already loaded, it returns the existing handle.
		void* probe_handle = dlopen(image_name, RTLD_LAZY);
		dlclose(probe_handle);
		
		if (handle == probe_handle)
		{
			return strdup(image_name);
		}
	}
	
	
	return NULL;
}




NSString *pathToCurrentExecutable()
{
	Dl_info info;
	
	
	if (dladdr(__builtin_return_address(0), &info))
	{
		return [NSString stringWithUTF8String: info.dli_fname];
	}
	else
	{
		lprintf("ERROR: Could not get path to current executable.\n");
		
		
		return NULL;
	}
}




char* path_to_current_executable()
{
	Dl_info info;
	
	
	if (dladdr(__builtin_return_address(0), &info))
	{
		return strdup(info.dli_fname);
	}
	else
	{
		lprintf("ERROR: Could not get path to current executable.\n");
		
		
		return NULL;
	}
}




NSString *getArg(int argc, char* argv[], char* argname)
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], argname) == 0)
		{
			if ((i + 1) < argc)
			{
				char* argvalue = argv[i+1];
				
				
				return [NSString stringWithUTF8String:argvalue];
			}
		}
	}
	
	
	return NULL;
}




char* getarg(int argc, char* argv[], char* argname) //TODO - make enough to match all argv definitions
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], argname) == 0)
		{
			if ((i + 1) < argc)
			{
				return argv[i + 1];
			}
		}
	}
	
	
	return NULL;
}




bool argIsPresent(int argc, char* argv[], NSString *argName) //TODO - make enough to match all argv definitions
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], [argName UTF8String]) == 0)
		{
			return true;
		}
	}
	
	
	return false;
}




bool arg_is_present(int argc, char* argv[], char* argname) //TODO - make enough to match all argv definitions
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], argname) == 0)
		{
			return true;
		}
	}
	
	
	return false;
}




// change the prefix - effects entire program, so please only call at the beginning
void set_lprefix(char* new_prefix)
{
	lprefix = strdup(new_prefix);
	logPrefix = [NSString stringWithUTF8String: new_prefix];
}




// basically just printf with a prefix & newline - 'l' for 'log'
void lprintf(const char* format, ...)
{
	char* wrappedString = malloc(strlen(lprefix) + strlen(format) + strlen("\n") + 1);
	
	
	// Wrap it between prefix and newline
	strcpy(wrappedString, lprefix);
	strcat(wrappedString, format);
	strcat(wrappedString, "\n");
	
	// Actual processing & printing
	va_list args;
	va_start(args, format); //maybe?
	
	vprintf(wrappedString, args);
	
	va_end(args);
}




void setLogPrefix(NSString *newPrefix)
{
	logPrefix = [newPrefix copy];
	lprefix = strdup([newPrefix UTF8String]);
}




void TMLog(NSString *format, ...)
{
	// Type to hold information about variable arguments.
	va_list ap;
	
	// Initialize a variable argument list.
	va_start (ap, format);
	
	// NSLog only adds a newline to the end of the NSLog format if
	// one is not already there.
	// Here we are utilizing this feature of NSLog()
	if (![format hasSuffix: @"\n"])
	{
		format = [format stringByAppendingString: @"\n"];
	}
	
	NSString *body = [[NSString alloc] initWithFormat:format arguments:ap];
	
	// End using variable argument list.
	va_end (ap);
	
	
	fprintf(stderr, "%s%s", [logPrefix UTF8String], [body UTF8String]);
}




void printclr()
{
	if (strcmp(getenv("TERM"), "dumb") == 0) // could also use isatty()
	{
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		
		printf ("lines %d\n", w.ws_row);
		int console_height = w.ws_row;
		int clear_height = console_height * 2;
		
		for (int i = 0; i < clear_height; i++)
		{
			printf("\n");
		}
	}
	else
	{
		printf("\033[H\033[J");
	}
}




#if defined(__x86_64__)
void pass_array_contents_to_variadic_function(long long function, int array_size, void* array, unsigned long array_type_size)
{
	/* TODO: Get rid of this part */
	// some nonesence to keep size from being overwritten
	int *local_size = malloc(sizeof(int));
	memcpy(local_size, &array_size, sizeof(int));
	free(local_size);
	
	
	__asm__ __volatile__ ("push %%rax\n\t"
						  "push %%rdx\n\t"
						  "push %%rcx\n\t"
						  : : :);
	
	// all to stack first to minimize chance of clang overwriting registers later
	switch (array_type_size)
	{
		case sizeof(BYTE):
		{
			for (int i = (array_size - 1); i > -1; i--)
			{
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("mov %0, %%r13" : : "m"(((BYTE*)array)[i]) :);
				__asm__ __volatile__ ("push %%r13\n\t" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
			}
			break;
		}
			
		case sizeof(WORD):
		{
			// this has had the least testing - you can probably trust it, but be skeptical
			for (int i = (array_size - 1); i > -1; i--)
			{
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("mov %0, %%r13" : : "m"(((WORD*)array)[i]) :);
				__asm__ __volatile__ ("push %%r13\n\t" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
			}
			break;
		}
			
		case sizeof(DWORD):
		{
			for (int i = (array_size - 1); i > -1; i--)
			{
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("movq %0, %%r13" : : "m"(((DWORD*)array)[i]) :);
				__asm__ __volatile__ ("pushq %%r13\n\t" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
			}
			break;
		}
			
		case sizeof(QWORD):
		{
			for (int i = (array_size - 1); i > -1; i--)
			{
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("push %0\n\t" : : "r"(((QWORD*)array)[i]) : );
				__asm__ __volatile__ ("nop" : : : );
				__asm__ __volatile__ ("nop" : : : );
			}
			break;
		}
			
		default:
		{
			__asm__ __volatile__ ("pop %%rcx\n\t"
								  "pop %%rdx\n\t"
								  "pop %%rax\n\t"
								  : : :);
			printf("VA_HAXX: Object size larger than supported\n");
			// TODO: Do one or the other based on debugger?
			//			__asm__("int $3");
			raise(SIGABRT);
			break;
		}
	}
	
	
	// loops are bad for registers, switch doesn't handle greater than case well (it would be default, but what if size is less than? etc)
	if (array_size == 1)
	{
		__asm__ __volatile__ ("popq %%rdi\n\t" : : : );
	}
	else if (array_size == 2)
	{
		__asm__ __volatile__ ("popq %%rdi\n\t" : : : );
		__asm__ __volatile__ ("popq %%rsi\n\t" : : : );
	}
	else if (array_size == 3)
	{
		__asm__ __volatile__ ("pop %%rdi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rsi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rdx\n\t" : : : );
	}
	else if (array_size == 4)
	{
		__asm__ __volatile__ ("pop %%rdi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rsi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rdx\n\t" : : : );
		__asm__ __volatile__ ("pop %%rcx\n\t" : : : );
	}
	else if (array_size == 5)
	{
		__asm__ __volatile__ ("pop %%rdi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rsi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rdx\n\t" : : : );
		__asm__ __volatile__ ("pop %%rcx\n\t" : : : );
		__asm__ __volatile__ ("pop %%r8\n\t" : : : );
	}
	else if (array_size >= 6)
	{
		__asm__ __volatile__ ("pop %%rdi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rsi\n\t" : : : );
		__asm__ __volatile__ ("pop %%rdx\n\t" : : : );
		__asm__ __volatile__ ("pop %%rcx\n\t" : : : );
		__asm__ __volatile__ ("pop %%r8\n\t" : : : );
		__asm__ __volatile__ ("pop %%r9\n\t" : : : );
	}
	
	
	__asm__ __volatile__ ("call *%P0" : :"r"(function) :);
	
	
	for (int i = 0; i < (array_size - 6); i++)
	{
		__asm__ __volatile__ ("nop" : : : );
		__asm__ __volatile__ ("nop" : : : );
		__asm__ __volatile__ ("pop %%r8" : : :);
		__asm__ __volatile__ ("nop" : : : );
		__asm__ __volatile__ ("nop" : : : );
	}
	
	
	__asm__ __volatile__ ("pop %%rcx\n\t"
						  "pop %%rdx\n\t"
						  "pop %%rax\n\t"
						  : : :);
}
#endif

