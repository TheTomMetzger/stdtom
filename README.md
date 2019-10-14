> Some includes are missing - update with fixes and new features coming soon
# stdtom
A collection of useful functions, some with common use cases, but many have especially obscure use cases.

An attempt at cross platform support was made with C and C++ versions, but it is mostly untested (some testing was done on Linux, but not fully tested).

### Language Support
Currently supported languages: C, C++, Objective-C, Objective-C++

More languages may be added, but only as needed by me.

<br>

## Functions
| Family Name | Description | Theoretical Universal Support
|-------------|---------------|-------------|
| `select_bit_range_from_word` | Selects a rang of bits from a `WORD`. | YES |
| `hex_to_decimal` | Converts a hex string to an integer. | YES |
| `strtonum` | Converts a string to a number. | YES |
| `string_with_format` | Makes a string according to a format. | YES |
| `strapp` | Appends one string to another. | YES |
| `str_append_char` | Appends a character to a string. | YES |
| `strtrim` | Trims a string to just the exact size it needs. | YES |
| `zero_str` | Zero's out every byte of a string. | YES |
| `str_is_0d` | Checks if a string has been zero'd. | YES |
| `str_from_file_contents` | Makes a string from the contents of a file. | YES |
| `path_from_handle` | Gets the path to the executable relate to a handle to the executable. | NO - Apple Exclusive
| `path_to_current_executable` | Gets the path to whatever executable calls to function. | YES |
| `getarg` | Gets the value that follows an argument (like `getopt`, but reasonable). | YES |
| `arg_is_present` | Checks if an argument is present. | YES |
| `set_lprefix` | Sets the prefix used by the print/log function. | YES |
| `lprintf` | Prints to the console, with a user-define prefix added first. | YES |
| `setLogPrefix` | Sets the prefix used by the print/log function. | NO - Apple Exclusive |
| `TMLog` | Prints to the console, with a user-define prefix added first. | NO - Apple Exclusive |
| `va_haxx` / `pass_array_contents_to_variadic_function` | Passes all elements of an array as individual arguments to a variadic function | YES - x86_64 Exclusive (for now!) | 

Overloads for all possible string types are provided as reasonable. If there's not an overload, then it's likely because there's some support for the operation to that type already.

I'm aware there's some projects that bring Objective-C to Linux (and possibly Windows) - so some of the "Apple Exclusives" may actually have wider support.

<br>

## Type Definitions
| Name | Description | Theoretical Universal Support
|-------------|---------------|-------------|
| `BYTE`      | An unsigned 8-bit value | YES |
| `WORD`      | An unsigned 16-bit value | YES |
| `DWORD`     | An unsigned 32-bit value | YES |
| `QWORD`     | An unsigned 64-bit value | YES |

<br>

## Special Definitions

| Family Name | Description | Theoretical Universal Support
|-------------|---------------|-------------|
| `NOTUSED()` | Silences compiler complaints about unused variables | YES |

There are some commented out debug functions that build on `lprintf`. <br>I haven't decided If these are actually worth having yet, and thus remain commented out.
