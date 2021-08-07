#include <string.h>
#include <stdio.h>
#include "stdbool.h"
#include <assert.h>

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include <pcre2.h>
#include "pcre_result.h"
#include "pcre_result_array_list.h"

struct Matches {
    size_t offset;
    pcre2_code *pCode;
    pcre2_match_data *matchData;
    uint32_t ovectorCount;
    PCRE2_SPTR subject;
    size_t subjectLen;
};

struct ErrorInfo {
    int errCode;
    size_t errOffset;
    const char *errMsg;
};

struct MatchResult {
    union {
        struct Matches matches;
        struct ErrorInfo errorInfo;
    };
    bool error;
};

struct Compiled {
    pcre2_code *pCode;
};

struct CompileResult {
    union {
        struct Compiled compiled;
        struct ErrorInfo errorInfo;
    };
    bool error;
};

struct CompileResult compile(const char *pattern) {
    int errCode;
    size_t errOffset;
    pcre2_code *pCode = pcre2_compile((PCRE2_SPTR) pattern, PCRE2_ZERO_TERMINATED, PCRE2_UTF, &errCode, &errOffset,
                                      NULL);
    if (pCode == NULL) {
        const char *errMsg = (const char *) malloc(1024 * (PCRE2_CODE_UNIT_WIDTH / 8));
        // todo: PCRE2_CODE_UNIT_WIDTH is not 8
        pcre2_get_error_message(errCode, (PCRE2_UCHAR8 *) errMsg, 1024);
        struct CompileResult r = {
                .errorInfo = {
                        .errCode = errCode,
                        .errOffset = errOffset,
                        .errMsg = errMsg
                },
                .error = true
        };
        return r;
    }

    struct CompileResult r = {
            .compiled = {
                    .pCode = pCode
            },
            .error = false
    };
    return r;
}

struct MatchResult match(const char *pattern, const char *subject) {
    struct CompileResult compileResult = compile(pattern);
    if (compileResult.error) {
        struct ErrorInfo errorInfo = compileResult.errorInfo;
        struct MatchResult r = {
                .errorInfo = errorInfo,
                .error = true
        };
        return r;
    }

    struct Compiled compiled = compileResult.compiled;
    pcre2_match_data *matchData = pcre2_match_data_create_from_pattern(compiled.pCode, NULL);

    struct MatchResult r = {
            .matches = {
                    .pCode = compiled.pCode,
                    .offset = 0,
                    .matchData = matchData,
                    .ovectorCount = pcre2_get_ovector_count(matchData),
                    .subject = (PCRE2_SPTR) subject,
                    .subjectLen = strlen(subject)
            },
            .error = false
    };
    return r;
}

struct Match {
    bool none;
    size_t *ovector;
    size_t length;
};

struct Match match_next(struct Matches *matches) {
    int status = pcre2_match(matches->pCode,
                             matches->subject,
                             matches->subjectLen,
                             matches->offset, 0,
                             matches->matchData,
                             NULL);
    if (status == PCRE2_ERROR_NOMATCH) {
        struct Match m = {
                .ovector = NULL,
                .length = 0,
                .none = true
        };
        return m;
    }
    size_t *ovector = pcre2_get_ovector_pointer(matches->matchData);
    size_t sub = ovector[1] - ovector[0];
    if (sub == 0) {
        struct Match m = {
                .none = true,
                .ovector = NULL,
                .length = 0
        };
        return m;
    }
    matches->offset += sub;
    struct Match m = {
            .ovector = ovector,
            .length = matches->ovectorCount * 2,
            .none = false
    };
    return m;
}

struct Group match_group(struct Match match, size_t group) {
    assert(!match.none);
    size_t startIndex = group * 2;
    size_t endIndex = startIndex + 1;
    assert(startIndex >= 0 && endIndex < match.length);
    size_t start = match.ovector[startIndex];
    size_t end = match.ovector[endIndex];

    struct Group r = {
            .start = start,
            .length = end - start
    };
    return r;
}

size_t match_group_length(struct Match match) {
    return match.length / 2;
}

void matches_free(struct Matches matches) {
    pcre2_match_data_free(matches.matchData);
    pcre2_code_free(matches.pCode);
}

void error_info_free(struct ErrorInfo errorInfo) {
    free((void *) errorInfo.errMsg);
}

void match_result_free(struct MatchResult result) {
    if (result.error) {
        error_info_free(result.errorInfo);
    }
    if (!result.error) {
        matches_free(result.matches);
    }
}

struct MatchListResult {
    union {
        struct ArrayList list;
        struct ErrorInfo errorInfo;
    };
    bool error;
};

struct MatchListResult match_to_list(const char *pattern, const char *subject) {
    struct MatchResult result = match(pattern, subject);
    if (result.error) {
        struct ErrorInfo errorInfo = result.errorInfo;
        struct MatchListResult list = {
                .errorInfo = errorInfo,
                .error = true
        };
        return list;
    }

    assert(!result.error);
    struct Matches matches = result.matches;

    struct ArrayList list;
    array_list_init_vp(&list);

    while (true) {
        struct Match next = match_next(&matches);
        if (next.none) {
            break;
        }
        assert(match_group_length(next) >= 1);
        struct Group group = match_group(next, 0);
        array_list_add_vp(&list, group);
    }

    match_result_free(result);
    struct MatchListResult r = {
            .list = list,
            .error = false
    };
    return r;
}

void match_list_result_free(struct MatchListResult result) {
    if (result.error) {
        error_info_free(result.errorInfo);
    }
    if (!result.error) {
        array_list_free_vp(&result.list);
    }
}

struct TestResult {
    bool error;
    union {
        bool match;
        struct ErrorInfo errorInfo;
    };
};

struct TestResult test(const char *pattern, const char *text) {
    struct MatchResult result = match(pattern, text);
    if (result.error) {
        struct TestResult r = {
                .errorInfo = result.errorInfo,
                .error = true
        };
        return r;
    }

    struct Matches matches = result.matches;

    struct Match next = match_next(&matches);
    if (next.none) {
        struct TestResult r = {
                .error = false,
                .match = false
        };
        return r;
    }

    matches_free(matches);

    struct TestResult r = {
            .error = false,
            .match = true
    };
    return r;
}

int test_test() {
    struct TestResult result = test(".", "ab");
    if (result.error) {
        struct ErrorInfo errorInfo = result.errorInfo;
        printf("%s\n", errorInfo.errMsg);
        error_info_free(errorInfo);
        return errorInfo.errCode;
    }

    printf("%i\n", result.match);
    return 0;
}

int main() {
    const char *subject = "Cmake 1.23.34宝贝，妈妈怀里安睡.";
    char *pattern = "(?<=Cmake )[0-9]+\\.[0-9]+\\.[0-9]+.+(?=\\.)";
    struct MatchListResult result = match_to_list(pattern, subject);
    if (result.error) {
        struct ErrorInfo errorInfo = result.errorInfo;
        printf("msg: %s, errOffset: %zu\n", errorInfo.errMsg, errorInfo.errOffset);
        match_list_result_free(result);
        return errorInfo.errCode;
    }

    struct ArrayList list = result.list;
    size_t length = array_list_length_vp(&list);
    for (int i = 0; i < length; ++i) {
        struct Group group = array_list_get_vp(&list, i);

        size_t k = group.start;
        for (size_t j = 0; j < group.length; ++j) {
            putchar(subject[k]);
            ++k;
        }
        putchar('\n');
    }

    match_list_result_free(result);

    return test_test();
}
