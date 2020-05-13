#pragma once

#define R_TRY(rc_expr)       \
    ({                       \
        auto rc = (rc_expr); \
        if (R_FAILED(rc))    \
            return rc;       \
    })

#define R_THROW(rc_expr)     \
    ({                       \
        auto rc = (rc_expr); \
        if (R_FAILED(rc))    \
            throw rc;        \
    })

#define THROW_UNLESS(expr, obj) \
    ({                          \
        bool res = (expr);      \
        if (!res)               \
            throw obj;          \
    })
