#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

char *parse_negative(char *string)
{
    if ( string[0] == '-')
        return string + 1;
    else
        return string;
}

char *parse_int(char *string)
{
    if ( string[0] == '\0') {
        return NULL;
    }
    else if ( string[0] == '0') {
        return string + 1;
    }
    else if ( string[0] >= '1' && string[0] <= '9') {
        string++;
        while (string[0] >= '0' && string[0] <= '9') {
            string++;
        }
        return string;
    }
    else {
        return NULL;
    }

}

char *parse_frac(char *string) {
    int c;
    if ( *string == '.')
    {
        string++;
        if ( (c = *string) >= '0' && c <= '9')
        {
            while((c = *string) >= '0' && c <= '9')
            {
                string++;
            }
            return string;
        }
        else {
            return NULL;
        }
    }
    else {
        return string;
    }
}

char *parse_exp(char *string) {
    int c;
    if ( (c = *string ) == 'e' || c == 'E') {
        string++;
        if ( *string == '\0')
            return NULL;
        if ( (c = *string) == '-' || c == '+') {
            string++;
        }
        if ( *string == '\0')
            return NULL;
        
        if ((c = *string) >= '0' && c <= '9')
        {
            while ((c = *string) >= '0' && c <= '9')
                string++;
            return string;
        }
        else {
            return NULL;
        }
    }
    else {
        return string;
        }
}

char *parse_number(char *string)
{
    if ( *string == '\0')
        return NULL;
    string = parse_negative(string);
    string = parse_int(string);
    if ( string == NULL)
        return NULL;
    string = parse_frac(string);
    if ( string == NULL)
        return NULL;
    string = parse_exp(string);
    if ( string == NULL)
        return NULL;
    return string;


}
static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    char *json = c->json;
    if ((json = parse_number(json)) == NULL)
        return LEPT_PARSE_INVALID_VALUE;
    /*printf("%s,%s\n", c->json, json);    */

    if (*json != '\0' && *json != ' ' && *json != '\t' && *json != '\n' && *json != '\r') 
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    printf("%s, %f\n", c->json, v->n);
    if ( errno == ERANGE &&(v->n == HUGE_VAL) || (v->n == -1 * HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

