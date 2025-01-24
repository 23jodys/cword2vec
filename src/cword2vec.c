#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#incldue <stdint.h>
#include <sds.h>
#define MAX_WORDS 5000

char* words[MAX_WORDS];
int word_count = 0;


typedef struct CenterContext {
	
}

uint64_t corpus_size = 2;

sds corpus[] = {
	sdsnew("the cat sat on the mat"),
	sdsnew("the dog barked at the cat")
};

/*
int index_words(char* corpus[], char* words[]) {

}
*/

int string_hash(char* string) {
	return atoi(string);
}

int main(int argc, char* argv[]) {
	char* sentence;
	word_count = 10;
	for (int i = 0; i < word_count; i++) {
		printf("%d", i);
		
	}
}
