/*
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "config.h"

#include "osdep/io.h"
#include "osdep/threads.h"

#include "common/common.h"
#include "common/global.h"
#include "common/msg.h"
#include "input/input.h"
#include "libmpv/client.h"
#include "options/m_config.h"
#include "options/options.h"
#include "options/path.h"
#include "player/client.h"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

struct mp_ipc_ctx {
    struct mp_log *log;
    struct mp_client_api *client_api;
    const char *path;

    pthread_t thread;
    int death_pipe[2];
};

struct client_arg {
    struct mp_log *log;
    struct mpv_handle *client;

    char *client_name;
    int client_fd;
    bool close_client_fd;

    bool writable;
};

static int ipc_write_str(struct client_arg *client, const char *buf)
{
    size_t count = strlen(buf);
    while (count > 0) {
        ssize_t rc = send(client->client_fd, buf, count, MSG_NOSIGNAL);
        if (rc <= 0) {
            if (rc == 0)
                return -1;

            if (errno == EBADF || errno == ENOTSOCK) {
                client->writable = false;
                return 0;
            }

            if (errno == EINTR || errno == EAGAIN)
                continue;

            return rc;
        }

        count -= rc;
        buf   += rc;
    }

    return 0;
}

static void *client_thread(void *p)
{
    pthread_detach(pthread_self());

    // We don't use MSG_NOSIGNAL because the moldy fruit OS doesn't support it.
    struct sigaction sa = { .sa_handler = SIG_IGN, .sa_flags = SA_RESTART };
    sigfillset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);

    int rc;

    struct client_arg *arg = p;
    bstr client_msg = { talloc_strdup(NULL, ""), 0 };

    mpthread_set_name(arg->client_name);

    int pipe_fd = mpv_get_wakeup_pipe(arg->client);
    if (pipe_fd < 0) {
        MP_ERR(arg, "Could not get wakeup pipe\n");
        goto done;
    }

    MP_VERBOSE(arg, "Client connected\n");

    struct pollfd fds[2] = {
        {.events = POLLIN, .fd = pipe_fd},
        {.events = POLLIN, .fd = arg->client_fd},
    };

    fcntl(arg->client_fd, F_SETFL, fcntl(arg->client_fd, F_GETFL, 0) | O_NONBLOCK);

    while (1) {
        rc = poll(fds, 2, 0);
        if (rc == 0)
            rc = poll(fds, 2, -1);
        if (rc < 0) {
            MP_ERR(arg, "Poll error\n");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            mp_flush_wakeup_pipe(pipe_fd);

            while (1) {
                mpv_event *event = mpv_wait_event(arg->client, 0);

                if (event->event_id == MPV_EVENT_NONE)
                    break;

                if (event->event_id == MPV_EVENT_SHUTDOWN)
                    goto done;

                if (!arg->writable)
                    continue;

                char *event_msg = mp_json_encode_event(event);
                if (!event_msg) {
                    MP_ERR(arg, "Encoding error\n");
                    goto done;
                }

                rc = ipc_write_str(arg, event_msg);
                talloc_free(event_msg);
                if (rc < 0) {
                    MP_ERR(arg, "Write error (%s)\n", mp_strerror(errno));
                    goto done;
                }
            }
        }

        if (fds[1].revents & (POLLIN | POLLHUP)) {
            while (1) {
                char buf[128];
                bstr append = { buf, 0 };

                ssize_t bytes = read(arg->client_fd, buf, sizeof(buf));
                if (bytes < 0) {
                    if (errno == EAGAIN)
                        break;

                    MP_ERR(arg, "Read error (%s)\n", mp_strerror(errno));
                    goto done;
                }

                if (bytes == 0) {
                    MP_VERBOSE(arg, "Client disconnected\n");
                    goto done;
                }

                append.len = bytes;

                bstr_xappend(NULL, &client_msg, append);

                while (bstrchr(client_msg, '\n') != -1) {
                    char *reply_msg = mp_ipc_consume_next_command(arg->client,
                        NULL, &client_msg);

                    if (reply_msg && arg->writable) {
                        rc = ipc_write_str(arg, reply_msg);
                        if (rc < 0) {
                            MP_ERR(arg, "Write error (%s)\n", mp_strerror(errno));
                            talloc_free(reply_msg);
                            goto done;
                        }
                    }

                    talloc_free(reply_msg);
                }
            }
        }
    }

done:
    if (client_msg.len > 0)
        MP_WARN(arg, "Ignoring unterminated command on disconnect.\n");
    talloc_free(client_msg.start);
    if (arg->close_client_fd)
        close(arg->client_fd);
    mpv_destroy(arg->client);
    talloc_free(arg);
    return NULL;
}

static void ipc_start_client(struct mp_ipc_ctx *ctx, struct client_arg *client)
{
    client->client = mp_new_client(ctx->client_api, client->client_name);
    if (!client->client)
        goto err;

    client->log = mp_client_get_log(client->client);

    pthread_t client_thr;
    if (pthread_create(&client_thr, NULL, client_thread, client))
        goto err;

    return;

err:
    if (client->client)
        mpv_destroy(client->client);

    if (client->close_client_fd)
        close(client->client_fd);

    talloc_free(client);
}

static void ipc_start_client_json(struct mp_ipc_ctx *ctx, int id, int fd)
{
    struct client_arg *client = talloc_ptrtype(NULL, client);
    *client = (struct client_arg){
        .client_name = talloc_asprintf(client, "ipc-%d", id),
        .client_fd   = fd,
        .close_client_fd = true,

        .writable = true,
    };

    ipc_start_client(ctx, client);
}

static void ipc_start_client_text(struct mp_ipc_ctx *ctx, const char *path)
{
    int mode = O_RDONLY;
    int client_fd = -1;
    bool close_client_fd = true;
    bool writable = false;

    if (strcmp(path, "/dev/stdin") == 0) { // for symmetry with Linux
        client_fd = STDIN_FILENO;
        close_client_fd = false;
    } else if (strncmp(path, "fd://", 5) == 0) {
        char *end = NULL;
        client_fd = strtol(path + 5, &end, 0);
        if (!end || end == path + 5 || end[0]) {
            MP_ERR(ctx, "Invalid FD: %s\n", path);
            return;
        }
        close_client_fd = false;
        writable = true; // maybe
    } else {
        // Use RDWR for FIFOs to ensure they stay open over multiple accesses.
        struct stat st;
        if (stat(path, &st) == 0 && S_ISFIFO(st.st_mode))
            mode = O_RDWR;
        client_fd = open(path, mode);
    }
    if (client_fd < 0) {
        MP_ERR(ctx, "Could not open '%s'\n", path);
        return;
    }

    struct client_arg *client = talloc_ptrtype(NULL, client);
    *client = (struct client_arg){
        .client_name = "input-file",
        .client_fd   = client_fd,
        .close_client_fd = close_client_fd,
        .writable = writable,
    };

    ipc_start_client(ctx, client);
}

static void *ipc_thread(void *p)
{
    int rc;

    int ipc_fd;
    struct sockaddr_un ipc_un = {0};

    struct mp_ipc_ctx *arg = p;

    mpthread_set_name("ipc socket listener");

    MP_VERBOSE(arg, "Starting IPC master\n");

    ipc_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ipc_fd < 0) {
        MP_ERR(arg, "Could not create IPC socket\n");
        goto done;
    }

#if HAVE_FCHMOD
    fchmod(ipc_fd, 0600);
#endif

    size_t path_len = strlen(arg->path);
    if (path_len >= sizeof(ipc_un.sun_path) - 1) {
        MP_ERR(arg, "Could not create IPC socket\n");
        goto done;
    }

    ipc_un.sun_family = AF_UNIX,
    strncpy(ipc_un.sun_path, arg->path, sizeof(ipc_un.sun_path) - 1);

    unlink(ipc_un.sun_path);

    if (ipc_un.sun_path[0] == '@') {
        ipc_un.sun_path[0] = '\0';
        path_len--;
    }

    size_t addr_len = offsetof(struct sockaddr_un, sun_path) + 1 + path_len;
    rc = bind(ipc_fd, (struct sockaddr *) &ipc_un, addr_len);
    if (rc < 0) {
        MP_ERR(arg, "Could not bind IPC socket\n");
        goto done;
    }

    rc = listen(ipc_fd, 10);
    if (rc < 0) {
        MP_ERR(arg, "Could not listen on IPC socket\n");
        goto done;
    }

    MP_VERBOSE(arg, "Listening to IPC socket.\n");

    int client_num = 0;

    struct pollfd fds[2] = {
        {.events = POLLIN, .fd = arg->death_pipe[0]},
        {.events = POLLIN, .fd = ipc_fd},
    };

    while (1) {
        rc = poll(fds, 2, -1);
        if (rc < 0) {
            MP_ERR(arg, "Poll error\n");
            continue;
        }

        if (fds[0].revents & POLLIN)
            goto done;

        if (fds[1].revents & POLLIN) {
            int client_fd = accept(ipc_fd, NULL, NULL);
            if (client_fd < 0) {
                MP_ERR(arg, "Could not accept IPC client\n");
                goto done;
            }

            ipc_start_client_json(arg, client_num++, client_fd);
        }
    }

done:
    if (ipc_fd >= 0)
        close(ipc_fd);

    return NULL;
}

struct mp_ipc_ctx *mp_init_ipc(struct mp_client_api *client_api,
                               struct mpv_global *global)
{
    struct MPOpts *opts = mp_get_config_group(NULL, global, GLOBAL_CONFIG);

    struct mp_ipc_ctx *arg = talloc_ptrtype(NULL, arg);
    *arg = (struct mp_ipc_ctx){
        .log        = mp_log_new(arg, global->log, "ipc"),
        .client_api = client_api,
        .path       = mp_get_user_path(arg, global, opts->ipc_path),
        .death_pipe = {-1, -1},
    };
    char *input_file = mp_get_user_path(arg, global, opts->input_file);

    talloc_free(opts);

    if (input_file && *input_file)
        ipc_start_client_text(arg, input_file);

    if (!arg->path || !arg->path[0])
        goto out;

    if (mp_make_wakeup_pipe(arg->death_pipe) < 0)
        goto out;

    if (pthread_create(&arg->thread, NULL, ipc_thread, arg))
        goto out;

    return arg;

out:
    if (arg->death_pipe[0] >= 0) {
        close(arg->death_pipe[0]);
        close(arg->death_pipe[1]);
    }
    talloc_free(arg);
    return NULL;
}

void mp_uninit_ipc(struct mp_ipc_ctx *arg)
{
    if (!arg)
        return;

    (void)write(arg->death_pipe[1], &(char){0}, 1);
    pthread_join(arg->thread, NULL);

    close(arg->death_pipe[0]);
    close(arg->death_pipe[1]);
    talloc_free(arg);
}
