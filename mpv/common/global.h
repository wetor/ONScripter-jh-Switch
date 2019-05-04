#ifndef MPV_MPV_H
#define MPV_MPV_H

// This should be accessed by glue code only, never normal code.
// The only purpose of this is to make mpv library-safe.
// Think hard before adding new members.
struct mpv_global {
    struct mp_log *log;
    struct m_config_shadow *config;
    struct mp_client_api *client_api;
    char *configdir;
};

#endif
