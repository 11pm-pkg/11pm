#include <memory.h>
#include <string.h>
#include <yajl/yajl_parse.h>

#include "package.h"

struct xi_pkg_parse_state;

static int xi_pkg_parse_null(void *ctx);
static int xi_pkg_parse_bool(void *ctx, int value);
static int xi_pkg_parse_num(void *ctx, const char *value, size_t len);
static int xi_pkg_parse_str(void *ctx, const unsigned char *value, size_t len);
static int xi_pkg_parse_arys(void *ctx);
static int xi_pkg_parse_arye(void *ctx);
static int xi_pkg_parse_maps(void *ctx);
static int xi_pkg_parse_mape(void *ctx);
static int xi_pkg_parse_mapk(void *ctx, const unsigned char *key, size_t klen);

static char *mkstrcpy(const char *const src, int len);
static int mkbfromh(unsigned char *dst, const char *const hex, int len);
static int xi_hdlvsnhash(
        struct xi_pkg_parse_state *state,
        enum _xipkg_version_hashes hash_type,
        unsigned char *hash_dest, int hash_len,
        const char *const hex, int hex_len
);

enum xi_pkg_parse_state_id {
    XI_PKG_PARSE__KEY = 3,
    XI_PKG_PARSE_ID             = 1 << 1,
    XI_PKG_PARSE_NAME           = 1 << 2,
    XI_PKG_PARSE_INFO           = 1 << 3,
    XI_PKG_PARSE_INFO_KEY,
    XI_PKG_PARSE_INFO_HOMEPAGE  = 1 << 4,
    XI_PKG_PARSE_INFO_SUMMARY   = 1 << 5,
    XI_PKG_PARSE_INFO_DESC      = 1 << 6,
    XI_PKG_PARSE_VSN_ARY        = 1 << 7,
    XI_PKG_PARSE_VSN_ITEM
};
static const unsigned int XI_PKG_PARSE_MASK =
    (XI_PKG_PARSE_VSN_ARY << 1) - 1;

enum xi_vsn_parse_state_id {
    XI_VSN_PARSE__KEY = 3,
    XI_VSN_PARSE_SOURCE         = 1 << 1,
    XI_VSN_PARSE_SOURCE_KEY = 5,
    XI_VSN_PARSE_SOURCE_URL_ARY = 1 << 2,
    XI_VSN_PARSE_SOURCE_URL_ITEM,
    XI_VSN_PARSE_SOURCE_MIRR,
    XI_VSN_PARSE_SOURCE_HASH    = 1 << 3,
    XI_VSN_PARSE_SOURCE_HASH_MD5,
    XI_VSN_PARSE_SOURCE_HASH_SHA1,
    XI_VSN_PARSE_SOURCE_HASH_SHA256,
    XI_VSN_PARSE_SOURCE_HASH_SHA512,
    XI_VSN_PARSE_EXTRACT_DIR,
    XI_VSN_PARSE_CHANGE_DIR,
    XI_VSN_PARSE_INSTR          = 1 << 4,
    XI_VSN_PARSE_INSTR_KEY,
    XI_VSN_PARSE_INSTR_BUILD    = 1 << 5,
    XI_VSN_PARSE_INSTR_INSTALL  = 1 << 6,
    XI_VSN_PARSE_INSTR_TEST
};
static const unsigned int XI_VSN_PARSE_MASK =
    (XI_VSN_PARSE_INSTR_INSTALL << 1) - 2;

struct xi_pkg_parse_state {
    enum { XI_PARSE_PKG = 1 << 16, XI_PARSE_VSN = 1 << 17 } parse_which;
    union {
        unsigned int _value;
        enum xi_pkg_parse_state_id pkg;
        enum xi_vsn_parse_state_id vsn;
    } state_id, status;
    unsigned int ignore_depth;

    union {
        xipackage_t *pkg;
        xipkg_version_t *vsn;
    } dest;
    const char *const current_key;

    enum {
        XI_PARSE_INCORRECT_TYPE,
        XI_PARSE_INCORRECT_VALUE,
        XI_PARSE_MISSING_KEY,
    } err;

    struct xi_pkg_parse_state *prev;
};

static yajl_callbacks xi_pkg_parse_callbacks = {
    xi_pkg_parse_null, xi_pkg_parse_bool,
    NULL, NULL, xi_pkg_parse_num,
    xi_pkg_parse_str,
    xi_pkg_parse_maps, xi_pkg_parse_mapk, xi_pkg_parse_mape,
    xi_pkg_parse_arys, xi_pkg_parse_arye
};


xipackage_t *xipackage_from_json(const char *const json_data)
{
}


static int xi_pkg_parse_null(void *ctx)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This value is ignored
        return 1;
    }
    
    switch (state->parse_which | state->state_id._value) {
        // The spec currently forbids null
        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }
    return 1;
}

static int xi_pkg_parse_num(void *ctx, const char *value, size_t len)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This value is ignored
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        // No properties exist in the current
        // spec with a number type
        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }
    return 1;
}

static int xi_pkg_parse_bool(void *ctx, int value)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This value is ignored
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_MIRR:
            state->status.vsn |= XI_VSN_PARSE_SOURCE_MIRR;
            state->dest.vsn->source_info.mirror_friendly = value
                ? XIPKG_MIRROR_FRIENDLY
                : XIPKG_MIRROR_UNFRIENDLY;
            state->state_id.vsn = XI_VSN_PARSE_SOURCE_KEY;
            break;
        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }
    return 1;
}

static int xi_pkg_parse_str(void *ctx, const unsigned char *value, size_t len)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This value is ignored
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        case XI_PARSE_PKG | XI_PKG_PARSE_ID:
            state->status.pkg |= XI_PKG_PARSE_ID;
            state->state_id.pkg = XI_PKG_PARSE__KEY;
            break;
        case XI_PARSE_PKG | XI_PKG_PARSE_NAME:
            state->status.pkg |= XI_PKG_PARSE_NAME;
            state->state_id.pkg = XI_PKG_PARSE__KEY;
            break;
        case XI_PARSE_PKG | XI_PKG_PARSE_INFO_HOMEPAGE:
            state->status.pkg |= XI_PKG_PARSE_INFO_HOMEPAGE;
            state->state_id.pkg = XI_PKG_PARSE_INFO_KEY;
            break;
        case XI_PARSE_PKG | XI_PKG_PARSE_INFO_SUMMARY:
            state->status.pkg |= XI_PKG_PARSE_INFO_SUMMARY;
            state->state_id.pkg = XI_PKG_PARSE_INFO_KEY;
            break;
        case XI_PARSE_PKG | XI_PKG_PARSE_INFO_DESC:
            state->status.pkg |= XI_PKG_PARSE_INFO_DESC;
            state->state_id.pkg = XI_PKG_PARSE_INFO_KEY;
            break;
        case XI_PARSE_PKG | XI_PKG_PARSE_VSN_ITEM:
            break;

        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_URL_ITEM:
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_MIRR:
            if (len == 14   // strlen("ABSOLUTELY_NOT") == 14
                    && strncmp(value, "ABSOLUTELY_NOT", len)) {
                state->dest
                    .vsn->source_info.mirror_friendly
                    = XIPKG_MIRROR_NEVER;
            } else {
                state->err = XI_PARSE_INCORRECT_VALUE;
                return 0;
            }
            state->state_id.vsn = XI_VSN_PARSE_SOURCE_KEY;
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_HASH_MD5:
            return xi_hdlvsnhash(state, XIPKG_HASH_MD5,
                                 state->dest.vsn->source_info.hash_md5, 16,
                                 value, len);
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_HASH_SHA1:
            return xi_hdlvsnhash(state, XIPKG_HASH_SHA1,
                                 state->dest.vsn->source_info.hash_sha1, 20,
                                 value, len);
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_HASH_SHA256:
            return xi_hdlvsnhash(state, XIPKG_HASH_SHA256,
                                 state->dest.vsn->source_info.hash_sha256, 32,
                                 value, len);
        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE_HASH_SHA512:
            return xi_hdlvsnhash(state, XIPKG_HASH_SHA512,
                                 state->dest.vsn->source_info.hash_sha512, 64,
                                 value, len);
        case XI_PARSE_VSN | XI_VSN_PARSE_EXTRACT_DIR:
            state->state_id.vsn = XI_VSN_PARSE__KEY;
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_CHANGE_DIR:
            state->state_id.vsn = XI_VSN_PARSE__KEY;
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_INSTR_BUILD:
            state->status.vsn |= XI_VSN_PARSE_INSTR_BUILD;
            state->state_id.vsn = XI_VSN_PARSE_INSTR_KEY;
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_INSTR_INSTALL:
            state->status.vsn |= XI_VSN_PARSE_INSTR_INSTALL;
            state->state_id.vsn = XI_VSN_PARSE_INSTR_KEY;
            break;
        case XI_PARSE_VSN | XI_VSN_PARSE_INSTR_TEST:
            state->state_id.vsn = XI_VSN_PARSE_INSTR_KEY;
            break;

        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }

    return 1;
}

static int xi_pkg_parse_arys(void *ctx)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This array, and all nested values, are ignored
        state->ignore_depth += 1;
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        case XI_PARSE_PKG | XI_PKG_PARSE_VSN_ARY:
            state->state_id.pkg = XI_PKG_PARSE_VSN_ITEM;
            break;
        
        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }

    return 1;
}

static int xi_pkg_parse_arye(void *ctx)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // Leaving an ignored array
        state->ignore_depth -= 1;
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        case XI_PARSE_PKG | XI_PKG_PARSE_VSN_ITEM:
            state->status.pkg |= XI_PKG_PARSE_VSN_ARY;
            state->state_id.pkg = XI_PKG_PARSE__KEY;
            break;
        
        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }

    return 1;
}

static int xi_pkg_parse_maps(void *ctx)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // This map, and all nested values, are ignored
        state->ignore_depth += 1;
        return 1;
    }

    switch (state->parse_which | state->state_id._value) {
        case XI_PARSE_PKG:
            state->state_id.pkg = XI_PKG_PARSE__KEY;
            break;
        
        case XI_PARSE_PKG | XI_PKG_PARSE_INFO:
            state->state_id.pkg = XI_PKG_PARSE_INFO_KEY;
            break;

        case XI_PARSE_VSN:
            state->state_id.vsn = XI_VSN_PARSE__KEY;
            break;

        case XI_PARSE_VSN | XI_VSN_PARSE_INSTR:
            state->state_id.vsn = XI_VSN_PARSE_INSTR_KEY;
            break;

        case XI_PARSE_VSN | XI_VSN_PARSE_SOURCE:
            state->state_id.vsn = XI_VSN_PARSE_SOURCE_KEY;
            break;

        default:
            state->err = XI_PARSE_INCORRECT_TYPE;
            return 0;
    }

    return 1;
}

static int xi_pkg_parse_mape(void *ctx)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth) {
        // Leaving an ignored map
        state->ignore_depth -= 1;
        return 1;
    }

    return 1;
}

static int xi_pkg_parse_mapk(void *ctx, const unsigned char *key, size_t klen)
{
    struct xi_pkg_parse_state *state = *((struct xi_pkg_parse_state **) ctx);
    if (state->ignore_depth == 1) {
        // Just finished an ignored value; stop ignoring
        state->ignore_depth = 0;
    } else if (state->ignore_depth) {
        // Still ignoring this map
        return 1;
    }

    // TODO: Map keys

    return 1;
}

static char *mkstrcpy(const char *const src, int len)
{
    char *dst = (char *) malloc((size_t) len + 1);
    strncpy(dst, src, len);
    dst[len] = 0;
    return dst;
}

#define XI_BFROMH(hd) \
    hd >= '0' && hd <= '9'  ? \
        hd - '0'            : \
    hd >= 'A' && hd <= 'F'  ? \
        (hd - 'A') + 10     : \
    hd >= 'a' && hd <= 'f'  ? \
        (hd - 'a') + 10     : \
        0xFF
static int mkbfromh(unsigned char *dst, const char *const hex, int len)
{
    if (len % 2) { return -1; }

    for (int i = 0; i < (len / 2); i += 1) {
        unsigned char hi = XI_BFROMH(hex[i*2]),
                      lo = XI_BFROMH(hex[i*2 + 1]);
        if (hi == 0xFF) { return i*2; }
        if (lo == 0xFF) { return i*2 + 1; }

        dst[i] = (hi << 4) | lo;
    }

    return 0;
}

static int xi_hdlvsnhash(
        struct xi_pkg_parse_state *state,
        enum _xipkg_version_hashes hash_type,
        unsigned char *hash_dest, int hash_len,
        const char *const hex, int hex_len
)
{
    if (hex_len != (hash_len * 2)) {
        state->err = XI_PARSE_INCORRECT_VALUE;
        return 0;
    }
    if (mkbfromh(hash_dest, hex, hex_len)) {
        state->err = XI_PARSE_INCORRECT_VALUE;
        return 0;
    }
    state->dest.vsn->source_info.hashes |= hash_type;
    state->status.vsn |= XI_VSN_PARSE_SOURCE_HASH;
    state->state_id.vsn = XI_VSN_PARSE_SOURCE_KEY;
    return 1;
}
