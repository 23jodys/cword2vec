#ifndef CWORD2VEC_H
#define CWORD2VEC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sds.h>

#include "dbg.h"

#define MAX_WORDS 5000
#define MAX_CORPUS 5000
#define CONTEXT 3

typedef uint64_t CwVocabIndex;
typedef struct CwVocab {
    CwVocabIndex max_index;
    CwVocabIndex max_size;
    sds* words;
} CwVocab;

typedef struct CwVocabResult {
    bool status;
    CwVocabIndex index;
    sds token;
} CwVocabResult;

CwVocabResult* cw_vocab_get(CwVocab* vocabulary, sds string, CwVocabResult* result);
CwVocabResult* cw_vocab_get_n(CwVocab* vocabulary, CwVocabIndex index, CwVocabResult* result);
CwVocabResult* cw_vocab_set(CwVocab* vocabulary, char* string, size_t string_len, CwVocabResult* result);
CwVocab* cw_vocab_init(CwVocabIndex max_size);
CwVocabResult* cw_vocab_result_init(void);

typedef struct Pair {
	CwVocabIndex center;
	CwVocabIndex context;
} Pair;

typedef struct CwCenterContext {
    CwVocabIndex center;
    Pair context_pairs[CONTEXT * 2];
} CwCenterContext;

typedef struct CwContextResult {
    bool status;
} CwContextResult;

typedef struct __attribute__((packed)) posix_header {   /* byte offset */
	char name[100];     // Filename
	char mode[8];       // File mode (octal)
	char uid[8];        // Owner's numeric user ID (octal)
	char gid[8];        // Group numeric ID (octal)
	char size[12];      // File size in bytes (octal)
	char mtime[12];     // Modification time (octal)
	char checksum[8];   // Checksum for header validation
	char typeflag[1];   // File type
	char linkname[100]; // Target name of link (if applicable)
	char magic[6];      // UStar indicator ("ustar")
	char version[2];    // UStar version
	char uname[32];     // Owner username
	char gname[32];     // Owner group name
	char devmajor[8];   // Major device number
	char devminor[8];   // Minor device number
	char prefix[155];   // Filename prefix
	char padding[12];   // Padding to make header 512 bytes
} Header;           /* 512 */

CwContextResult* cw_context_result_init(void);
CwContextResult* cw_context_pairs_make(CwVocab* vocabulary, sds filename, CwContextResult* result);
int _parse_corpus(char* buffer, size_t size, CwVocab* vocabulary);

typedef struct CwTrainingContext {
	size_t n;
	size_t len;
	struct CwCenterContext* data;
} CwTrainingContext;


CwTrainingContext* cw_training_context_init(void); 
CwTrainingContext* cw_training_context_make(CwTrainingContext* context, size_t n); 

#endif
