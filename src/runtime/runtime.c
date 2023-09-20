#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>



// builtin functions for tiger.
void __print__(char *s) {
    printf("%s", s);
}

void __puti__(__int64_t i) {
    printf("%lld", i);
}

void __flush__() {
    fflush(stdout);
}

char* __getchar__() {
    char *ch = (char*)malloc(2 * sizeof(char));
    ch[1] = '\0';
    if(scanf("%c", &ch[0]) != EOF)
        return ch;
    ch[0] = '\0';
    return ch;
}

__int64_t __getint__() {
    char buffer[100]; // assuming number input won't exceed 100 characters
    char *endptr;

    if (!fgets(buffer, sizeof(buffer), stdin)) {
        // handle error or exit
        perror("Failed to read input");
        exit(EXIT_FAILURE);
    }

    errno = 0; // reset error number
    __int64_t i = strtoll(buffer, &endptr, 10); // base 10 conversion

    // Check for various possible errors
    if ((errno == ERANGE && (i == LONG_LONG_MAX || i == LONG_LONG_MIN))
        || (errno != 0 && i == 0)) {
        perror("Conversion error");
        exit(EXIT_FAILURE);
    }

    if (endptr == buffer) {
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }

    return i;
}

__int64_t __ord__(char *s) {
    if(s[0] == '\0')
        return -1;
    return (int)s[0];
}

char* __chr__(__int64_t i) {
    if(i < 0 || i > 127) {
        printf("runtime error: chr(%lld)\n", i);
        exit(1);
    }
    char *ch = (char*)malloc(2 * sizeof(char));
    ch[1] = '\0';
    ch[0] = (char)i;
    return ch;
}

__int64_t __size__(char *s) {
    size_t len = strlen(s);
    if (len > LLONG_MAX) {
        fprintf(stderr, "String length exceeds __int64_t capacity.\n");
        exit(EXIT_FAILURE);
    }
    return (__int64_t)len;
}

char* __substring__(char *s, __int64_t first, __int64_t n) {
    if((first + n) > strlen(s)) {
        printf("runtime error: substring len %lu, start %lld, n %lld\n", strlen(s), first, n);
        exit(1);
    }
    char *substr = (char*)malloc((n+1) * sizeof(char));
    substr[n] = '\0';
    memcpy(substr, s+first, n);
    return substr;
}

char* __concat__(char *s1, char *s2) {
    __int64_t l1 = strlen(s1);
    __int64_t l2 = strlen(s2);
    char *res = (char*)malloc((l1 + l2 + 1) * sizeof(char));
    res[l1+l2] = '\0';
    memcpy(res, s1, l1);
    memcpy(res + l1, s2, l2);
    return res;
}

__int64_t __not__(__int64_t i) {
    return i == 0;
}

void __exit__(__int64_t i) {
    exit(i);
}

char* __gets__() {
    char *res = (char*)malloc(256);
    scanf("%s", res);
    return res;
}

void *alloc(__int64_t sz) {
    return malloc(sz);
}