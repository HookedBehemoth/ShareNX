#pragma once

#define R_TRY(rc_expr)       \
    ({                       \
        auto rc = (rc_expr); \
        if (R_FAILED(rc))    \
            return rc;       \
    })

#define R_STR(cmd, message) \
    ({                      \
        auto rc = (cmd);    \
        if (R_FAILED(rc))   \
            return message; \
    })

#define R_THROW(rc_expr)     \
    ({                       \
        auto rc = (rc_expr); \
        if (R_FAILED(rc))    \
            throw rc;        \
    })

#define R_THROW_UNLESS(rc_expr, obj) \
    ({                        \
        auto rc = (rc_expr);  \
        if (R_FAILED(rc))     \
            throw obj;        \
    })

#define THROW_UNLESS(expr, obj) \
    ({                          \
        bool res = (expr);      \
        if (!res)               \
            throw obj;          \
    })
