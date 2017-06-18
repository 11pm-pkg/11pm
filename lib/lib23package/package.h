#pragma once
/* package.h - functions and data types primarily
 * related to packages and package lists (but
 * notably not containing any install-related
 * functions) */


enum _xipkg_command_type {
    XIPKG_COMMAND_EXEC,
    XIPKG_COMMAND_SHELL
};

enum _xipkg_mirror_friendly {
    XIPKG_MIRROR_FRIENDLY = 0,
    XIPKG_MIRROR_UNFRIENDLY,
    XIPKG_MIRROR_NEVER
};

enum _xipkg_version_hashes {
    XIPKG_HASH_MD5      = 1 << 0,
    XIPKG_HASH_SHA1     = 1 << 1,
    XIPKG_HASH_SHA256   = 1 << 2,
    XIPKG_HASH_SHA512   = 1 << 3,
};


struct _xipkg_step_info {
    const char *const workdir;
    enum _xipkg_command_type command_type;
    const char *const command;
};

typedef struct xipkg_version {
    const char *const vid;

    struct {
        const char *const *const urls;
        enum _xipkg_mirror_friendly mirror_friendly;
        enum _xipkg_version_hashes hashes;
        unsigned char hash_md5[16];
        unsigned char hash_sha1[20];
        unsigned char hash_sha256[32];
        unsigned char hash_sha512[64];
    } source_info;

    const char *const extract_dir;

    struct {
        struct _xipkg_step_info build;
        struct _xipkg_step_info install;
        struct _xipkg_step_info test;
    } steps;
} xipkg_version_t;

typedef struct xipackage {
    const char *const id;
    const char *const name;
    struct {
        const char *const homepage;
        const char *const summary;
        const char *const description;
    } info;
    const xipkg_version_t *const versions;
} xipackage_t;


xipackage_t *xipackage_from_json(const char *const json_data);

void xipackage_free(xipackage_t *package);
