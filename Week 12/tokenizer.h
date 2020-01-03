#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <stdio.h>
#include "parser.h"

/* Mainly defined for a shorter version of unsigned int. However, also allows
 * buffer MAX to be changed easily. line_t must be unsigned for MAX macro to 
 * work
 */
typedef unsigned int line_t;

void loadFile(prog_t* program, char* filename);

line_t parseBufferWords(char** pos);

/* ------ FILE HANDLING FUNCTIONS ------ */
FILE* openFile(char* filename);

/* Reads line from a file. Function returns number of characters in the string
 * including \n or 0 on file end or error. Must be passed initialised buffer or
 * NULL value ptr
 */
line_t getLine(char** buffer, line_t* size, FILE* file);

/* Truncates line endings from lines. Can handle LF or CRLF. Uses short-circuit
 * evaluation to avoid reading outside array indices by checking for zero-length
 * strings and ending type. Does nothing to string if line ends aren't
 * found.
 */
void truncateLineEnd(char* buffer, line_t* len);

/* Handles malloc and reallocing buffer. On the initial call buffer should be 
 * passed as NULL. For resizing, the existing pointer should be passed.
 */
char* bufferAllocHandler(char* buffer, line_t size);

#endif
