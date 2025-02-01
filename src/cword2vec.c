#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cword2vec.h"

CwVocab* cw_vocab_init(CwVocabIndex max_size) {
	CwVocab* vocabulary = calloc(1, sizeof(CwVocab));
	vocabulary->words = calloc(max_size, sizeof(sds));
	vocabulary->max_index = 0;
	vocabulary->max_size = max_size;
	if (vocabulary->words == NULL) {
		log_err("wtf");
		exit(-1);
	}

	return vocabulary;
}

CwVocabResult* cw_vocab_result_init(void) {
	CwVocabResult* result = calloc(1, sizeof(CwVocabResult));
	result->index = 0;
	result->status = true;

	return result;
}

CwVocabResult* cw_vocab_get(CwVocab* vocabulary, sds string, CwVocabResult* result) {
	if (result->status == false) {
		return result;
	}

	/* do this extremely naively */
	bool found = false;
	for (int i = 0; i < vocabulary->max_index; i++) {
		if (0 == strcmp(string, vocabulary->words[i])) {
			result->index = i;
			found = true;
		}
	}

	result->status = found;

	return result;
}

CwVocabResult* cw_vocab_get_n(CwVocab* vocabulary, CwVocabIndex index, CwVocabResult* result) {
	if (result->status == false) { 
		return result;
	}

	if (index >= vocabulary->max_size) {
		result->status = false;
		return result;
	}

	if (index >= vocabulary->max_index) {
		result->status = false;
		return result;
	}

	result->token = vocabulary->words[index];

	return result;

}

CwVocabResult* cw_vocab_set(CwVocab* vocabulary, char* string, size_t string_len, CwVocabResult* result) {
	if (result->status == false) { 
		return result;
	}
	bool found = false;

	result->status = false;

	for (int i = 0; i < vocabulary->max_index; i++) {
		if (0 == strncmp(string, vocabulary->words[i], string_len)) {
			found = true;
			result->index = i;
			result->status = true;
		}
	}

	if (found == false) {
		if (vocabulary->max_index >= vocabulary->max_size) {
			log_err("out of word space, %llu >= %llu", vocabulary->max_index, vocabulary->max_size);
			result->status = false;
		} else {
			sds word = sdsnewlen(string, string_len);
			//debug("Adding '%s'", word);
			vocabulary->words[vocabulary->max_index] = word;
			result->index = vocabulary->max_index;
			vocabulary->max_index++;
			result->status = true;
		}
	} else {
		//sds word = sdsnewlen(string, string_len);
		//debug("Already added %s", word);
	}

	return result;
}

CwContextResult* cw_context_result_init(void) {
	CwContextResult* result = calloc(1, sizeof(CwContextResult));
	return result;
}


unsigned long octal_to_decimal(const char *str, size_t size) {
	char temp[16] = {0};  
	memcpy(temp, str, size);
        return strtoul(temp, NULL, 8);  // Convert octal to decimal
}

CwContextResult* cw_context_pairs_make(CwVocab* vocabulary, sds filename, CwContextResult* result) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		log_err("Couldn't open file");
		result->status = false;
		return result;
	}

	struct stat sb;
	if (fstat(fd, &sb) == -1) {
		log_err("couldn't fstat");
		close(fd);
		result->status = false;
		return result;
	}

	size_t filesize = sb.st_size;

	char* data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		log_err("mmap");
		close(fd);
		result->status = false;
		return result;
	}

	close(fd);

	char* ptr = (char*) data;
	uint64_t count = 0;

	while (ptr < (char*)data + filesize) {
		Header* header = (Header *)ptr;
		if (header->name[0] == '\0') {
			break;
		}

		// temp to make this debugabble
		if (count > 2) {
			break;
		}

		unsigned long entry_size = octal_to_decimal(header->size, sizeof(header->size));

		log_info("File: %s, Size: %lu", header->name, entry_size);
		size_t num_blocks = (entry_size + 512 - 1) / 512;	
		//char* inner_ptr = ptr + 512;
		ptr += 512 + (num_blocks * 512);
		count++;

		_parse_corpus(ptr + 512, num_blocks * 512, vocabulary);
	}

	result->status = true;
	return result;
}

/*  
 *   this |   is |    a |  test |  for  | token | making | test1 | test2 | test3|
 * 0  x      o      o
 * 1  o      x      o       o
 * 2  o      o      x       o       o
 * 3         o      o       x       o       o
 * 4                o       o       x       o        o
 * 5                        o       o       x        o       o
 * 6                                o       o        x       o        o
 * 7                                        o        o       x        o
 */

int _word(int n) {
	sds minus_2_word, minus_1_word, current_word, plus_1_word, plus_2_word; 



	return n;

}
CwTrainingContextFile* cw_training_context_file_create(sds filename, size_t requested_buffer_size, CwTrainingContextFile* result) {
	if (!result) {
		result = calloc(1, sizeof(result));
		result->status = true;
		result->filehandle = NULL;
		result->buffer = NULL;
		result->filename = NULL;
	}
	result->buffer = malloc(requested_buffer_size);
	check(result->buffer, "couldn't allocate buffer");
	result->buffer_size = requested_buffer_size;

	result->filehandle = fopen(filename, "wb");
	result->filename = filename;
	check(result->filehandle, "couldn't open file %s", filename);

	setvbuf(result->filehandle, result->buffer, _IOFBF, requested_buffer_size);
	return result;

error:
	result->filehandle = NULL;
	result->status = false;
	return result;
}

CwTrainingContextFile* cw_training_context_file_finish(CwTrainingContextFile* file) {
	check(file->filehandle, "Filehandle was NULL");
	fclose(file->filehandle);
	file->filehandle = NULL;
	free(file->buffer);
	file->buffer = NULL;
error:
	return file;

}

int _parse_corpus(char* buffer, size_t size, CwVocab* vocabulary) {
	// Create CenterContext for 2*context + 1 so we can create
	// center context pairs for all possible sentences as we scan 
	// through the text

	size_t last_start = 0;
	size_t word_count = 0;
	bool in_word = false;
	size_t word_count_in_sentence = 0;

	CwVocabIndex tokens_to_process[2];
	size_t tokens_to_process_count = 0;

	CwVocabResult* result = cw_vocab_result_init();
	
	for (size_t i=0; i < size; i++) {
		switch(buffer[i]) {
			// sentence boundaries
			case '.':
			case '!':
			case '?':
				if (in_word) {
					//add the word from last start
					result = cw_vocab_set(vocabulary, buffer + last_start, i - last_start, result);
					check(result->status, "failed setting vocab at %lu", i);
					tokens_to_process[0] = result->index; tokens_to_process_count++;
					word_count++;

					//add the token as well
					result = cw_vocab_set(vocabulary, buffer + i, 1, result);
					check(result->status, "failed setting vocab at %lu", i);
					tokens_to_process[1] = result->index; tokens_to_process_count++;
					word_count++;

					in_word = false;
				}
				break;
			case '\n':
				//debug("NON TOKEN end sentence");
				word_count_in_sentence = 0;
				break;
			case ' ':
				if (in_word) {
					//debug("' ' in_word false, last_start %zu, i %zu", last_start, i);
					in_word = false;
					word_count++;
					result = cw_vocab_set(vocabulary, buffer + last_start, i - last_start, result);
					check(result->status, "failed setting vocab at %lu", i);
					tokens_to_process[0] = result->index; tokens_to_process_count++;
				}
				break;
			default:
				if (!in_word) {
					//debug("* in_word false, last_start %zu, i %zu", last_start, i);
					in_word = true;
					last_start = i;
				}
				break;
		}
		for (int i = 0; i < tokens_to_process_count; i++) {
			debug("need to process: '%llu'", tokens_to_process[i]);			
			if (word_count_in_sentence == 0) {
				// We are at beginning of sentence, so allocate need context pairs
				// allocate CONTEXT * 2 plus 1 to startr
				CwTrainingContext* context = cw_training_context_init(CONTEXT * 2 + 1);

			}
		}
		tokens_to_process_count = 0;
	}
error:
	return word_count;

}

CwTrainingContext* cw_training_context_init(size_t count) {
	CwTrainingContext* result = calloc(1, sizeof(CwTrainingContext));
	check(result, "unable to alloc result");
	
       	result->data = calloc(count, sizeof(CwCenterContext));
	result->len = count;
	return result;
error:
	exit(-1);
}

CwTrainingContext* cw_training_context_create_new(CwTrainingContext* context) {
	check(context, "NULL context passed in");
	/* check if we have at least one available */
	if (context->n < context->len) {
		context->n++;
		return context;
	} else {
		/* oh, then lets allocate a bunch more space and return one */
		size_t new_len = (context->len) * 1.414;
		context->data = realloc(context->data, (sizeof(CwCenterContext) * new_len));
		check(context->data, "unable to alloc"); 
		context->len = new_len;
		context->n++;
		return context;
	}
error:
	exit(-1);

}
