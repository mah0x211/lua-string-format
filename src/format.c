/**
 *  Copyright (C) 2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
// lua
#include <lauxlib.h>
#include <lua.h>

static int push_string(lua_State *L)
{
    char *str = (char *)lua_topointer(L, 1);
    lua_pushstring(L, str);
    return 1;
}

/**
 * @brief is_utf8firstb determines whether b is the first byte of UTF-8
 * @param b byte to be checked whether it is the first byte of UTF-8 or not.
 * @return int 1 if b is the first byte of UTF-8, otherwise 0.
 */
static int is_utf8firstb(unsigned char b)
{
    switch (b) {
    case 0x00 ... 0x7F:
    case 0xC2 ... 0xDF:
    case 0xE0 ... 0xEF:
    case 0xF0 ... 0xF4:
        return 1;
    default:
        return 0;
    }
}

/**
 * @brief utf8len determines the length of UTF-8 character pointed to by s.
 * @param s pointer to UTF-8 character sequence to be checked. s must be NULL
 * terminated.
 * @return int length of UTF-8 character pointed to by s. If s does not point to
 * a valid UTF-8 character, it returns a negative length.
 */
static int utf8len(unsigned char *s)
{
    //
    // The Unicode Standard
    // Version 15.0 – Core Specification
    //
    // Chapter 3
    //  Conformance
    // https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf
    //
    // Table 3-7. Well-Formed UTF-8 Byte Sequences
    //
    // illegal byte sequence will be replaced with U+FFFD ("\xEF\xBF\xBD")
    //
#define is_utf8tail(c) (((c) & 0xC0) == 0x80)

    switch (*s) {
    case 0x00 ... 0x7F:
        // 1 byte: 00-7F
        return 1;

    case 0x80 ... 0xBF:
    case 0xC0 ... 0xC1:
        // illegal 1 byte sequence
        return -1;

    case 0xC2 ... 0xDF:
        // 2 byte: C2-DF 80-BF
        if (is_utf8tail(s[1])) {
            return 2;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        }
        return -2;

    case 0xE0:
        // 3 byte: E0 A0-BF 80-BF
        if (s[1] >= 0xA0 && s[1] <= 0xBF && is_utf8tail(s[2])) {
            return 3;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        }
        return -3;

    case 0xE1 ... 0xEC:
        // 3 byte: E1-EC 2(80-BF)
        if (is_utf8tail(s[1]) && is_utf8tail(s[2])) {
            return 3;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        }
        return -3;

    case 0xED:
        // 3 byte: ED 80-9F 80-BF
        if (s[1] >= 0x80 && s[1] <= 0x9F && is_utf8tail(s[2])) {
            return 3;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        }
        return -3;

    case 0xEE ... 0xEF:
        // 3 byte: EE-EF 2(80-BF)
        if (is_utf8tail(s[1]) && is_utf8tail(s[2])) {
            return 3;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        }
        return -3;

    case 0xF0:
        // 4 byte: F0 90-BF 2(80-BF)
        if (s[1] >= 0x90 && s[1] <= 0xBF && is_utf8tail(s[2]) &&
            is_utf8tail(s[3])) {
            return 4;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        } else if (is_utf8firstb(s[3])) {
            return -3;
        }
        return -4;

    case 0xF1 ... 0xF3:
        // 4 byte: F1-F3 3(80-BF)
        if (is_utf8tail(s[1]) && is_utf8tail(s[2]) && is_utf8tail(s[3])) {
            return 4;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        } else if (is_utf8firstb(s[3])) {
            return -3;
        }
        return -4;

    case 0xF4:
        // 4 byte: F4 80-8F 2(80-BF)
        if (s[1] >= 0x80 && s[1] <= 0x8F && is_utf8tail(s[2]) &&
            is_utf8tail(s[3])) {
            return 4;
        } else if (is_utf8firstb(s[1])) {
            return -1;
        } else if (is_utf8firstb(s[2])) {
            return -2;
        } else if (is_utf8firstb(s[3])) {
            return -3;
        }
        return -4;

#undef is_utf8tail

    default:
        // illegal byte sequence: F5-FF
        return -1;
    }
}

static const char *tolstring(lua_State *L, int idx, size_t *len)
{
    int type = 0;

    if (luaL_callmeta(L, idx, "__tostring")) {
        lua_replace(L, idx);
    }

    type = lua_type(L, idx);
    switch (type) {
    case LUA_TNUMBER:
    case LUA_TSTRING:
        lua_pushvalue(L, idx);
        break;

    case LUA_TNIL:
        lua_pushliteral(L, "nil");
        break;

    case LUA_TBOOLEAN:
        if (lua_toboolean(L, idx)) {
            lua_pushliteral(L, "true");
        } else {
            lua_pushliteral(L, "false");
        }
        break;

    // case LUA_TTABLE:
    // case LUA_TFUNCTION:
    // case LUA_TTHREAD:
    // case LUA_TUSERDATA:
    // case LUA_TLIGHTUSERDATA:
    default: {
        char b[BUFSIZ];
        size_t n = snprintf(b, BUFSIZ, "%s: %p", lua_typename(L, type),
                            lua_topointer(L, idx));
        lua_pushlstring(L, b, n);
    } break;
    }

    return lua_tolstring(L, -1, len);
}

static void push_quoted_string(lua_State *L, int arg_idx)
{
    int top          = lua_gettop(L);
    size_t len       = 0;
    unsigned char *s = (unsigned char *)tolstring(L, arg_idx, &len);
    luaL_Buffer b    = {};

    luaL_buffinit(L, &b);
    luaL_addchar(&b, '"');
    while (len > 0) {
        int nbyte = utf8len(s);
        if (nbyte < 0) {
            // invalid utf8 byte sequences will be replaced with U+FFFD
            luaL_addlstring(&b, "\xEF\xBF\xBD", 3);
            nbyte = -nbyte;
            // skip invalid utf8 byte sequences
            s += nbyte;
            len -= nbyte;
            continue;
        } else if (nbyte > 1) {
            // copy utf8 byte sequences
            luaL_addlstring(&b, (char *)s, nbyte);
            s += nbyte;
            len -= nbyte;
            continue;
        }
        len--;

        if (*s == '"' || *s == '\\') {
            luaL_addchar(&b, '\\');
            luaL_addchar(&b, *s);
        } else if (!iscntrl(*s)) {
            luaL_addchar(&b, *s);
        } else {
            switch (*s) {
            case 0:
                luaL_addstring(&b, "\\0");
                break;
            case 7:
                luaL_addstring(&b, "\\a");
                break;
            case 8:
                luaL_addstring(&b, "\\b");
                break;
            case 9:
                luaL_addstring(&b, "\\t");
                break;
            case 10:
                luaL_addstring(&b, "\\n");
                break;
            case 11:
                luaL_addstring(&b, "\\v");
                break;
            case 12:
                luaL_addstring(&b, "\\f");
                break;
            case 13:
                luaL_addstring(&b, "\\r");
                break;

            default: {
                char buf[10];
                if (!isdigit(*(s + 1))) {
                    snprintf(buf, sizeof(buf), "\\%d", (int)*s);
                } else {
                    snprintf(buf, sizeof(buf), "\\%03d", (int)*s);
                }
                luaL_addstring(&b, buf);
            } break;
            }
        }
        s++;
    }
    luaL_addchar(&b, '"');
    luaL_pushresult(&b);
    lua_replace(L, top + 1);
    lua_settop(L, top + 1);
    return;
}

static void push_format_string(lua_State *L, const char *fmt, int type,
                               int arg_idx)
{
    union {
        lua_Integer i;
        lua_Number d;
        const char *s;
        const void *p;
    } val;
    char *mem = NULL;
    int rc    = 0;

    switch (type) {
    case 'd': // int (decimal)
    case 'i': // int (decimal) (same as 'd')
    case 'o': // unsigned int (octal)
    case 'u': // unsigned int (decimal)
    case 'x': // unsigned int (hexadecimal)
    case 'X': // unsigned int (hexadecimal) (uppercase)
        if (lua_type(L, arg_idx) == LUA_TBOOLEAN) {
            val.i = lua_toboolean(L, arg_idx);
        } else {
            val.i = luaL_checkinteger(L, arg_idx);
        }
        if (asprintf(&mem, fmt, val.i) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'c': // int (character)
        if (lua_type(L, arg_idx) == LUA_TSTRING) {
            size_t slen   = 0;
            const char *s = lua_tolstring(L, arg_idx, &slen);
            if (slen > 1) {
                luaL_argerror(L, arg_idx, "string length <=1 expected");
            }
            val.i = *s;
        } else {
            val.i = luaL_checkinteger(L, arg_idx);
        }
        if (asprintf(&mem, fmt, val.i) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'e': // double (scientific)
    case 'E': // double (scientific) (uppercase)
    case 'f': // double (decimal)
    case 'F': // double (decimal) (uppercase)
    case 'g': // double (scientific or decimal)
    case 'G': // double (scientific or decimal) (uppercase)
    case 'a': // double (hexadecimal) (C99)
    case 'A': // double (hexadecimal) (C99) (uppercase)
        val.d = luaL_checknumber(L, arg_idx);
        if (asprintf(&mem, fmt, val.d) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 's': { // any (string)
        int top = lua_gettop(L);
        val.s   = tolstring(L, arg_idx, NULL);
        if (asprintf(&mem, fmt, val.s) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        lua_settop(L, top);
    } break;

    case 'p': // void * (pointer)
        val.p = lua_topointer(L, arg_idx);
        if (asprintf(&mem, fmt, val.p) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'q': // any (quoted string)
        if (fmt[0] != '%' || fmt[1] != 'q' || fmt[2]) {
            luaL_error(L, "specifier '%%q' cannot have modifiers");
        }
        push_quoted_string(L, arg_idx);
        return;
    }

    lua_pushcfunction(L, push_string);
    lua_pushlightuserdata(L, mem);
    rc = lua_pcall(L, 1, 1, 0);
    // free allocated string
    free(mem);

    if (rc != 0) {
        // rethrow error with error message in stack top
        lua_error(L);
    }
}

static inline int uint2str(lua_State *L, char *buf, size_t len,
                           const char *placeholder, const int narg, int idx)
{
    if (idx > narg) {
        luaL_error(L,
                   "not enough arguments for placeholder '%s' in format string",
                   placeholder);
    }

    luaL_checktype(L, idx, LUA_TNUMBER);
    // convert argument to string as integer
    return snprintf(buf, len, "%d", (int)lua_tonumber(L, idx));
}

/**
 * @brief format arguments and push them to the stack.
 * - format argments must be placed after format string.
 * - generated strings are pushed to the top of the stack.
 * - you can concat them as follows:
 *
 *   int top = lua_gettop(L);
 *   int lastargs = format_arguments(L, 1); // 1 is index of format string
 *   lua_concat(L, lua_gettop(L) - top); // concat formatted strings
 *
 * @param L lua state
 * @param fmt_idx index of format string
 * @return int index of last used argument. if equal to fmt_idx, no argument
 * was used.
 */
static int format_arguments(lua_State *L, const int fmt_idx)
{
    const int narg   = lua_gettop(L);
    const char *fmt  = NULL;
    const char *head = NULL;
    const char *cur  = NULL;
    int nextarg      = fmt_idx;

    if (lua_type(L, fmt_idx) != LUA_TSTRING) {
        // ignore non-string format string
        return 0;
    }
    fmt = head = cur = lua_tostring(L, fmt_idx);

    // parse format specifiers
    while (*cur) {
        if (*cur == '%') {
            char buf[255]     = {0};
            size_t blen       = sizeof(buf);
            char *placeholder = buf;

#define COPY2PLACEHOLDER(str, len)                                             \
    do {                                                                       \
        size_t slen = (len);                                                   \
        if (slen >= blen) {                                                    \
            return luaL_error(L,                                               \
                              "each placeholder must be less than %d "         \
                              "characters in format string '%s'",              \
                              sizeof(placeholder), placeholder);               \
        }                                                                      \
        blen -= slen;                                                          \
        memcpy(placeholder, (str), slen);                                      \
        placeholder += slen;                                                   \
    } while (0)

            if (cur[1] == '%') {
                lua_pushlstring(L, head, cur - head + 1);
                // skip '%%' escape sequence
                cur += 2;
                head = cur;
                continue;
            }

            // push leading format string
            if (cur != head) {
                lua_pushlstring(L, head, cur - head);
            }
            fmt  = cur;
            head = cur;
            cur++;

            // flags field
            while (strchr("#I0- +'", *cur)) {
                cur++;
            }

            // int n_bits = sizeof(int) * 8;
            // int max_digits = n_bits / 3;
            // int buffer_size = max_digits + 2 + 1;
#define DYNSIZE (sizeof(int) * CHAR_BIT / 3 + 3)

            // width field
            while (strchr("1234567890*", *cur)) {
                if (*cur == '*') {
                    int wlen              = DYNSIZE;
                    const char w[DYNSIZE] = {0};

                    // copy leading format string
                    COPY2PLACEHOLDER(head, cur - head);
                    // skip '*'
                    head = cur + 1;

                    // get width from argument
                    nextarg++;
                    wlen = uint2str(L, (char *)w, (size_t)wlen, fmt, narg,
                                    nextarg);
                    // copy it to placeholder
                    COPY2PLACEHOLDER(w, wlen);
                }
                cur++;
            }

            // precision field
            if (*cur == '.') {
                // skip '.'
                cur++;
                while (strchr("1234567890*", *cur)) {
                    if (*cur == '*') {
                        int wlen              = DYNSIZE;
                        const char w[DYNSIZE] = {0};

                        // copy leading format string
                        COPY2PLACEHOLDER(head, cur - head);
                        // skip '*'
                        head = cur + 1;

                        // get precision from argument
                        nextarg++;
                        wlen = uint2str(L, (char *)w, wlen, fmt, narg, nextarg);
                        // copy it to placeholder
                        COPY2PLACEHOLDER(w, wlen);
                    }
                    cur++;
                }
            }

#undef DYNSIZE

            // length modifier
            if (strchr("hljztL", *cur)) {
                cur++;
            }

            // type field
            if (!strchr("diouxXeEfFgGaAcspqm", *cur)) {
                return luaL_error(L,
                                  "unsupported type field at '%c' in "
                                  "format string '%s'",
                                  *cur, fmt);
            }

            // copy leading format string
            COPY2PLACEHOLDER(head, cur - head + 1);
            head = cur + 1;

            if (*cur == 'm') {
                // printf %m is printed as strerror(errno) without params
                lua_checkstack(L, 1);
                lua_pushstring(L, strerror(errno));
            } else {
                nextarg++;
                if (nextarg > narg) {
                    return luaL_error(L,
                                      "not enough arguments for placeholder "
                                      "'%s' in format string",
                                      buf);
                }
                push_format_string(L, buf, *cur, nextarg);
            }
        }
        cur++;
    }

#undef COPY2PLACEHOLDER

    // push trailing format string
    if (cur > head) {
        lua_pushlstring(L, head, cur - head);
    }

    // index of last used argument
    return nextarg;
}

static int format_lua(lua_State *L)
{
    const int narg = lua_gettop(L);
    int lastarg    = format_arguments(L, 1);
    int unused     = narg - lastarg;

    // concat all strings
    lua_concat(L, lua_gettop(L) - narg);

    if (unused > 0) {
        int tblidx = lastarg + 2;

        // place result string to the previous position of unused argument table
        lua_insert(L, lastarg + 1);
        // create table for unused arguments
        lua_createtable(L, unused, 0);
        lua_insert(L, tblidx);
        for (int i = unused; i > 0; i--) {
            // push top of stack to unused argument table
            lua_rawseti(L, tblidx, i);
        }
        lua_pushinteger(L, unused);
        return 3;
    }
    return 1;
}

LUALIB_API int luaopen_string_format(lua_State *L)
{
    lua_pushcfunction(L, format_lua);
    return 1;
}
