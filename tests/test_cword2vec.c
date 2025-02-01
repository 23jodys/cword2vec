#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cmocka.h>

#include <cword2vec.h>

static void test_basic(void** state) {
    sds array[2];
    array[0] = sdsnew("test1");
    array[1] = sdsnew("test2");
}

static void test_cw_vocab_set(void** state) {
    CwVocab* vocab = cw_vocab_init(MAX_WORDS); 

    CwVocabIndex index;
    sds word = sdsnew("quick");

    CwVocabResult* result = cw_vocab_result_init();

    result = cw_vocab_set(vocab, word, sdslen(word), result);
    assert_int_equal(result->index, 0);
    assert_int_equal(vocab->max_index, 1); 

    sds word_2 = sdsnew("quick");
    result = cw_vocab_set(vocab, word_2, sdslen(word_2), result);
    assert_int_equal(result->index, 0);
    assert_int_equal(vocab->max_index, 1); 

    sds word_3 = sdsnew("brown");
    result = cw_vocab_set(vocab, word_3, sdslen(word_3), result);
    assert_int_equal(result->index, 1);
    assert_int_equal(vocab->max_index, 2); 

}

static void test_cw_vocab_set_at_max(void** state) {

    CwVocab* vocab = cw_vocab_init(10); 

    debug("in test: max_size %llu", vocab->max_size);

    CwVocabResult* result = cw_vocab_result_init();

    for (int i=0; i < 10; i++) {
        sds word = sdsnew("quick");
        word = sdscatprintf(word, "%d", i);
        cw_vocab_set(vocab, word, sdslen(word), result);
    }

    result = cw_vocab_set(vocab, sdsnew("fuck"), 5, result);
    assert_false(result->status);
}

static void test_cw_vocab_get_n(void** state) {
    CwVocab* vocab = cw_vocab_init(10);
    CwVocabResult* result = cw_vocab_result_init();

    for (int i=0; i < 10; i++) {
        sds word = sdsnew("quick");
        word = sdscatprintf(word, "%d", i);
        cw_vocab_set(vocab, word, sdslen(word), result);
    }

    result = cw_vocab_get_n(vocab, 3, result);
    assert_string_equal("quick3", result->token);
}

static void test_cw_vocab_get_n_edge(void** state) {
    CwVocab* vocab = cw_vocab_init(10);
    CwVocabResult* result = cw_vocab_result_init();

    for (int i=0; i < 10; i++) {
        sds word = sdsnew("quick");
        word = sdscatprintf(word, "%d", i);
        cw_vocab_set(vocab, word, sdslen(word), result);
    }

    result = cw_vocab_get_n(vocab, 9, result);
    assert_string_equal("quick9", result->token);
}

static void test_cw_vocab_get_n_out_of_bounds(void** state) {
    CwVocab* vocab = cw_vocab_init(10);
    CwVocabResult* result = cw_vocab_result_init();

    for (int i=0; i < 10; i++) {
        sds word = sdsnew("quick");
        word = sdscatprintf(word, "%d", i);
        cw_vocab_set(vocab, word, sdslen(word), result);
    }

    result = cw_vocab_get_n(vocab, 10, result);
    assert_false(result->status);
}

static void test_cw_context_pairs_make(void** state) {
    CwVocab* vocab = cw_vocab_init(5000);
    CwContextResult* result = cw_context_result_init();
    sds filename = sdsnew("/Users/jodys/SynologyDrive/Downloads/openwebtext/urlsf_subset00-1_data");
    cw_context_pairs_make(vocab, filename, result); 
    
}

static void test__parse_corpus_1(void ** state) {
    CwVocab* vocab = cw_vocab_init(5000);
    sds buffer = sdsnew("This is a test sentence.");
    _parse_corpus(buffer, sdslen(buffer), vocab);
    CwVocabResult* result = cw_vocab_result_init();
    result = cw_vocab_get(vocab, "test", result);
    assert_int_equal(3, result->index);
}

static void test__parse_corpus_2(void ** state) {
    CwVocab* vocab = cw_vocab_init(5000);
    sds buffer = sdsnew("This is a test sentence.  There were two   spaces!?!?This is supposed to be a CHALLENGE");
    _parse_corpus(buffer, sdslen(buffer), vocab);
    CwVocabResult* result = cw_vocab_result_init();
    result = cw_vocab_get(vocab, "be", result);
    assert_int_equal(13, result->index);
}

static void test__cw_training_context_create_new_1(void ** state) {
	CwTrainingContext* context = cw_training_context_init(7);

	context = cw_training_context_create_new(context);
	assert_int_equal(1, context->n);

	context = cw_training_context_create_new(context);
	assert_int_equal(2, context->n);
}

static void test_cw_training_context_file_basic(void** state) {
	CwTrainingContextFile* result;
	result = cw_training_context_file_create("./test.dat", 64 * 1024, NULL);
	assert_true(result->status);
	assert_int_equal(result->buffer_size, 1024 * 64);
	result = cw_training_context_file_finish(result);
	assert_null(result->filehandle);
	assert_null(result->buffer);

}

int main(int argc, char* argv[]) {

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_basic),
                cmocka_unit_test(test_cw_vocab_set),
                cmocka_unit_test(test_cw_vocab_get_n),
                cmocka_unit_test(test_cw_vocab_get_n_edge),
                cmocka_unit_test(test_cw_vocab_get_n_out_of_bounds),
                cmocka_unit_test(test_cw_vocab_set_at_max),
                cmocka_unit_test(test_cw_context_pairs_make),
                cmocka_unit_test(test__parse_corpus_1),
                cmocka_unit_test(test__parse_corpus_2),
		cmocka_unit_test(test__cw_training_context_create_new_1),
		cmocka_unit_test(test_cw_training_context_file_basic),
	};
	cmocka_run_group_tests(tests, NULL, NULL);
}
