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

#include "stdtom.hpp"

#include <sys/cdefs.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include <sys/ioctl.h>
#include <signal.h>




#ifndef __APPLE__ // For consistent behavior across platforms
#define ARG_MAX            (256 * 1024)
#endif


#define	INVALID		1
#define	TOOSMALL	2
#define	TOOLARGE	3





namespace tom
{
	string lprefix = "";
	
	
	
	
	// inspired by answers to this stack overflow question: https://stackoverflow.com/questions/10090326/how-to-extract-specific-bits-from-a-number-in-c
	short select_bit_range_from_word(WORD source, int from, int through)
	{
		short selection;
		
		
		through++; // the function does things "to", not "through"
		
		
		// average of the top two answers
		selection = (source >> from) & ((1 << (through - from)) - 1);
		
		
		return selection;
	}
	
	
	
	
	int hextodec(string hex_string)
	{
		int hex;
		sscanf(hex_string.c_str(), "%x", &hex);
		
		
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
	int strtonum(string numstr, int minval, int maxval, string* errstrp)
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
			num = (int)std::strtol(numstr.c_str(), &ep, 10);
			if (errno == EINVAL || numstr == ep || *ep != '\0')
				error = INVALID;
			else if ((num == INT_MIN && errno == ERANGE) || num < minval)
				error = TOOSMALL;
			else if ((num == INT_MAX && errno == ERANGE) || num > maxval)
				error = TOOLARGE;
		}
		if (errstrp != NULL)
		{
			char* everrstr = ev[error].errstr; // we may also want to change this to an strdup...
			if (everrstr == NULL)
			{
				errstrp->assign("");
			}
			else
			{
				errstrp->assign(everrstr);
			}
		}
		errno = ev[error].err;
		if (error)
			num = 0;
		
		return (num);
	}
	
	
	
	
	int cstrtonum(char *numstr, int minval, int maxval, char **errstrp)
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
	
	
	
	
	// utilizes elements from lprintf
	string string_with_format(string format, ...)
	{
		string formatted;
		
		char* raw_format;
		char raw_formatted[ARG_MAX]; // resuing ARG_MAX because it's the largest string we can expect
		
		
		// copy the string to a c string
		raw_format = (char* )malloc((sizeof(char) * format.size()) + 1);
		
		strcpy(raw_format, format.c_str());
		
		
		// Actual processing
		va_list args;
		va_start(args, format);
		
		vsnprintf(raw_formatted, ARG_MAX, raw_format, args);
		
		va_end(args);
		
		
		formatted = raw_formatted;
		
		
		return formatted;
	}
	
	
	
	
	char* cstring_with_format(char* format, ...)
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
	char* cstrapp(const char* s1, const char* s2) //strapp = string append
	{
		char* result = (char*)malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
		// in real code you would check for errors in malloc here
		// TODO: Check malloc errors
		strcpy(result, s1);
		strcat(result, s2);
		
		
		return result;
	}
	
	
	
	
	// One small caveat - if the string isn't big enough this will break
	void cstr_append_char(char* str, char character)
	{
		long length = strlen(str);
		str[length] = character;
		str[(length + 1)] = '\0';
	}
	
	
	
	
	// trims strings down to their real size. Also removes newline characters.
	char* cstrtrim (char* str)
	{
		char* trimmed = (char*)malloc((sizeof(char) * strlen(str)));
		memcpy(trimmed, str, (strlen(str) + 1)); //strlen doesn't count the null byte
		
		
		//no, actually we dont.
		//	//we want to trim newlines too - if they are present
		//	if (trimmed[(strlen(trimmed) - 1)] == '\n')
		//	{
		//		trimmed[(strlen(trimmed) - 1)] = '\0';
		//
		//		char* newtered = malloc((sizeof(char) * (strlen(trimmed)))); // no need to subtract one now, because strlen will stop at the new '\0'
		//		memcpy(newtered, trimmed, (strlen(trimmed)));
		//
		//
		//		return strdup(newtered); //strdup may not be what we want?
		//	}
		//	else
		//	{
		return strdup(trimmed); //strdup may not be what we want?
		//	}
	}
	
	
	
	
	// zeros out c string
	void zero_cstr(char** str)
	{
		memset(*str, 0, strlen(*str));
	}
	
	
	
	
	// checks if c string is all 0'd out
	//TODO make it check for pattern
	bool cstr_is_0d(char* str)
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
	
	
	
	
	string str_from_file_contents(string filename)
	{
		string filecontents = "";
		char* contents = NULL;
		
		
		FILE* file_handle = fopen(filename.c_str(), "r");
		
		if (file_handle != NULL)
		{
			fseek(file_handle, 0L, SEEK_END);
			long length = ftell(file_handle);
			
			rewind(file_handle);
			
			contents = (char*) malloc(length);
			
			if (contents != NULL)
			{
				fread(contents, length, 1, file_handle);
				fclose(file_handle);
#ifdef __linux__
				file_handle = NULL; // linux apparently doesn't do this for you?
#endif
				
			}
		}
		
		
		// cleanup
		if (file_handle != NULL)
		{
			fclose(file_handle);
		}
		
		
		contents[strlen(contents)] = '\0';
		
		filecontents += contents;
		
		
		return filecontents; //maybe need to dup?
	}
	
	
	
	
	char* cstr_from_file_contents(char* filename)
	{
		char* contents = NULL;
		
		
		FILE* file_handle = fopen(filename, "r");
		
		if (file_handle != NULL)
		{
			fseek(file_handle, 0L, SEEK_END);
			long length = ftell(file_handle);
			
			rewind(file_handle);
			
			contents = (char*) malloc(length);
			
			if (contents != NULL)
			{
				fread(contents, length, 1, file_handle);
				fclose(file_handle);
#ifdef __linux__
				file_handle = NULL; // linux apparently doesn't do this for you?
#endif
				
			}
		}
		
		
		// cleanup
		if (file_handle != NULL)
		{
			fclose(file_handle);
		}
		
		
		contents[strlen(contents)] = '\0';
		
		
		return strdup(contents);
	}
	
	
	
	
	// There's definitely a more standard C++ way to do this via pos & substring but whatever
	string path_extension(string path)
	{
		string extension;
		
		
		const char *dot = strrchr(path.c_str(), '.');
		if(!dot || dot == path) return "";
		
		
		extension = strdup(dot + 1);
		
		
		return extension;
	}
	
	
	
	
	char* path_extension(char* path)
	{
		const char *dot = strrchr(path, '.');
		if(!dot || dot == path) return "";
		
		
		return strdup(dot + 1);
	}
	
	
	
	
	bool isdir(string path)
	{
		struct stat myFile;
		
		
		if (stat(path.c_str(), &myFile) < 0)
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
	size_t filesize(string file_path)
	{
		struct stat st;
		
		
		stat(file_path.c_str(), &st);
		
		
		return st.st_size;
	}
	
	
	
	
	
	size_t filesize(char* file_path)
	{
		struct stat st;
		
		
		stat(file_path, &st);
		
		
		return st.st_size;
	}
	
	
	
	
#ifdef __APPLE__
	// Based on https://www.boost.org/doc/libs/1_63_0/boost/dll/detail/posix/path_from_handle.hpp
	// NOT a thread safe way of doing things
	string path_from_handle(void* handle)
	{
		// Since we know the image we want will always be near the end of the list, start there and go backwards
		for (uint32_t i = (_dyld_image_count() - 1); i >= 0; i--)
		{
			string image_name = _dyld_get_image_name(i);
			
			// Why dlopen doesn't effect _dyld stuff: if an image is already loaded, it returns the existing handle.
			void* probe_handle = dlopen(image_name.c_str(), RTLD_LAZY);
			dlclose(probe_handle);
			
			if (handle == probe_handle)
			{
				return image_name;
			}
		}
		
		
		return NULL;
	}
	
	
	
	
	const char* path_from_handle_cstr(void* handle)
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
				return image_name;
			}
		}
		
		
		return NULL;
	}
	
	
	
	
	
	string path_to_current_executable()
	{
		Dl_info info;
		
		
		if (dladdr(__builtin_return_address(0), &info))
		{
			string path = info.dli_fname;
			return path;
		}
		else
		{
			lprintf("ERROR: Could not get path to current executable.\n");
			
			
			return NULL;
		}
	}
	
	
	
	
	const char* path_to_current_executable_cstr()
	{
		Dl_info info;
		
		
		if (dladdr(__builtin_return_address(0), &info))
		{
			return info.dli_fname;
		}
		else
		{
			lprintf("ERROR: Could not get path to current executable.\n");
			
			
			return NULL;
		}
	}
#endif
	
	
	
	
	string getarg(int argc, char* argv[], string argname) //TODO - make enough to match all argv definitions
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], argname.c_str()) == 0)
			{
				if ((i + 1) < argc)
				{
					string argvalue = argv[i+1];
					
					
					return argvalue;
				}
			}
		}
		
		
		return "";
	}
	
	
	
	
	string getarg(int argc, char* argv[], char* argname) //TODO - make enough to match all argv definitions
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], argname) == 0)
			{
				if ((i + 1) < argc)
				{
					string argvalue = argv[i+1];
					
					
					return argvalue;
				}
			}
		}
		
		
		return "";
	}
	
	
	
	
	char* getarg_cstr(int argc, char* argv[], char* argname) //TODO - make enough to match all argv definitions
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
	
	
	
	
	bool arg_is_present(int argc, char* argv[], string argname) //TODO - make enough to match all argv definitions
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], argname.c_str()) == 0)
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
	void set_lprefix(string new_prefix)
	{
		lprefix = new_prefix;//strdup(new_prefix);
	}
	
	
	
	
	// basically just printf with a prefix & newline - 'l' for 'log'
	void lprintf(const char* format, ...)
	{
		char* wrappedString = (char*) malloc(strlen(lprefix.c_str()) + strlen(format) + strlen("\n") + 1);
		
		
		// Wrap it between prefix and newline
		strcpy(wrappedString, lprefix.c_str());
		strcat(wrappedString, format);
		strcat(wrappedString, "\n");
		
		// Actual processing & printing
		va_list args;
		va_start(args, format); //maybe?
		
		vprintf(wrappedString, args);
		
		va_end(args);
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
}
