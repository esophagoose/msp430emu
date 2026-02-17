#include "../main.h"
#include <sys/socket.h>
#include <sys/un.h>

static bool g_ipc_enabled = false;
static int g_ipc_server_fd = -1;
static int g_ipc_client_fd = -1;
static char g_ipc_path[sizeof(((struct sockaddr_un *)0)->sun_path)] = {0};

static void ipc_try_accept_client(void)
{
    int fd;

    if (!g_ipc_enabled || g_ipc_server_fd < 0 || g_ipc_client_fd >= 0)
    {
        return;
    }

    fd = accept(g_ipc_server_fd, NULL, NULL);
    if (fd >= 0)
    {
        g_ipc_client_fd = fd;
    }
}

static void ipc_send_line(const char *line)
{
    ssize_t wrote;
    size_t len;

    if (!g_ipc_enabled || line == NULL)
    {
        return;
    }

    ipc_try_accept_client();
    if (g_ipc_client_fd < 0)
    {
        return;
    }

    len = strlen(line);
    wrote = write(g_ipc_client_fd, line, len);

    if (wrote < 0)
    {
        close(g_ipc_client_fd);
        g_ipc_client_fd = -1;
    }
}

bool ipc_configure(const char *spec, char *err, size_t err_len)
{
    struct sockaddr_un addr;
    const char *path;

    if (err != NULL && err_len > 0)
    {
        err[0] = 0;
    }

    if (spec == NULL || strncmp(spec, "unix:", 5) != 0)
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "only unix: endpoints are supported");
        }
        return false;
    }

    path = spec + 5;
    if (*path == 0)
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "missing socket path");
        }
        return false;
    }

    if (strlen(path) >= sizeof(addr.sun_path))
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "socket path too long");
        }
        return false;
    }

    ipc_close();

    g_ipc_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_ipc_server_fd < 0)
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "socket() failed: %s", strerror(errno));
        }
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    strncpy(g_ipc_path, path, sizeof(g_ipc_path) - 1);

    unlink(path);

    if (bind(g_ipc_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "bind() failed: %s", strerror(errno));
        }
        ipc_close();
        return false;
    }

    if (listen(g_ipc_server_fd, 1) < 0)
    {
        if (err != NULL && err_len > 0)
        {
            snprintf(err, err_len, "listen() failed: %s", strerror(errno));
        }
        ipc_close();
        return false;
    }

    g_ipc_enabled = true;
    return true;
}

void ipc_close(void)
{
    if (g_ipc_client_fd >= 0)
    {
        close(g_ipc_client_fd);
        g_ipc_client_fd = -1;
    }

    if (g_ipc_server_fd >= 0)
    {
        close(g_ipc_server_fd);
        g_ipc_server_fd = -1;
    }

    if (g_ipc_path[0] != 0)
    {
        unlink(g_ipc_path);
        g_ipc_path[0] = 0;
    }

    g_ipc_enabled = false;
}

void ipc_emit_gpio(Emulator *emu, uint8_t port, uint8_t pin, uint8_t dir, uint8_t value)
{
    char line[160] = {0};

    if (emu == NULL)
    {
        return;
    }

    snprintf(line, sizeof(line),
             "{\"type\":\"gpio\",\"port\":%u,\"pin\":%u,\"dir\":%u,\"value\":%u,\"pc\":\"0x%04X\"}\n",
             port, pin, dir, value, emu->cpu->pc);
    ipc_send_line(line);
}

void ipc_emit_uart_tx(Emulator *emu, uint8_t value)
{
    char line[160] = {0};
    uint16_t pc = 0;

    if (emu != NULL)
    {
        pc = emu->cpu->pc;
    }

    snprintf(line, sizeof(line),
             "{\"type\":\"uart_tx\",\"value\":%u,\"pc\":\"0x%04X\"}\n",
             value, pc);
    ipc_send_line(line);
}

void print_console(Emulator *emu, const char *buf)
{
    (void)emu;
    (void)buf;
}

void print_serial(Emulator *emu, char *buf)
{
    size_t i;

    if (buf != NULL)
    {
        for (i = 0; buf[i] != 0; i++)
        {
            ipc_emit_uart_tx(emu, (uint8_t)buf[i]);
        }
        printf("%s", buf);
        fflush(stdout);
    }
}

void send_control(Emulator *emu, uint8_t opcode, void *data, size_t size)
{
    (void)emu;
    (void)opcode;
    (void)data;
    (void)size;
}
