#include "translation/translation.hpp"

#include <borealis.hpp>
#include <borealis/crash_frame.hpp>
#include <cstring>
#include <string>
#include <switch.h>
#include <util/fmt.hpp>

extern "C" {

    alignas(16) u8 __nx_exception_stack[0x8000];
    u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
    void __libnx_exception_handler(ThreadExceptionDump *ctx);
    u32 __nx_applet_exit_mode;
    u32 __nx_applet_type = AppletType_LibraryApplet;

}

brls::CrashFrame *g_crashFrame = nullptr;

static void unwindStack(u64 *outStackTrace, s32 *outStackTraceSize, size_t maxStackTraceSize, u64 currFp) {
    struct StackFrame {
        u64 fp; // Frame Pointer (Pointer to previous stack frame)
        u64 lr; // Link Register (Return address)
    };

    for (size_t i = 0; i < maxStackTraceSize; i++) {
        if (currFp == 0 || currFp % sizeof(u64) != 0)
            break;

        auto currTrace                        = reinterpret_cast<StackFrame *>(currFp);
        outStackTrace[(*outStackTraceSize)++] = currTrace->lr;
        currFp                                = currTrace->fp;
    }
}

static void printDebugInfo(ThreadExceptionDump *ctx, uintptr_t base_address) {
    u64 stackTrace[0x20] = {0};
    s32 stackTraceSize   = 0;
    unwindStack(stackTrace, &stackTraceSize, 0x20, ctx->fp.x);

    brls::Logger::error("[PC] BASE + 0x%lx", ctx->pc.x - base_address);
    brls::Logger::error("[LR] BASE + 0x%lx", ctx->lr.x - base_address);
    brls::Logger::error("[SP] BASE + 0x%lx", ctx->sp.x - base_address);
    brls::Logger::error("[FP] BASE + 0x%lx", ctx->fp.x - base_address);
    brls::Logger::error("== Begin stack trace ==");
    for (s8 i = 0; i < stackTraceSize - 2; i++)
        brls::Logger::error("[%d] BASE + 0x%lx", stackTraceSize - i - 3, stackTrace[i] - base_address);
    brls::Logger::error("== End stack trace ==");
}

void __libnx_exception_handler(ThreadExceptionDump *ctx) {
    thread_local static bool alreadyCrashed = false;
    static ThreadExceptionDump ctxBackup;

    u32 p;
    MemoryInfo info;
    svcQueryMemory(&info, &p, (u64)&__libnx_exception_handler);
    u64 base_address = info.addr;

    if (alreadyCrashed) {
        brls::Logger::error("Fatal exception thrown during exception handling. Closing immediately.");

        // Setup FatalCpuContext to pass on crash information to fatal
        static FatalCpuContext fatalCtx = {0};
        for (u8 i = 0; i < 29; i++)
            fatalCtx.aarch64_ctx.x[i] = ctxBackup.cpu_gprs[i].x;

        fatalCtx.aarch64_ctx.pc = ctxBackup.pc.x;
        fatalCtx.aarch64_ctx.lr = ctxBackup.lr.x;
        fatalCtx.aarch64_ctx.sp = ctxBackup.sp.x;
        fatalCtx.aarch64_ctx.fp = ctxBackup.fp.x;

        fatalCtx.aarch64_ctx.start_address = base_address;
        unwindStack(fatalCtx.aarch64_ctx.stack_trace, reinterpret_cast<s32 *>(&fatalCtx.aarch64_ctx.stack_trace_size), 32, ctxBackup.fp.x);
        fatalCtx.aarch64_ctx.afsr0              = ctxBackup.afsr0;
        fatalCtx.aarch64_ctx.afsr1              = ctxBackup.afsr1;
        fatalCtx.aarch64_ctx.esr                = ctxBackup.esr;
        fatalCtx.aarch64_ctx.far                = ctxBackup.far.x;
        fatalCtx.aarch64_ctx.register_set_flags = 0xFFFFFFFF;
        fatalCtx.aarch64_ctx.pstate             = ctxBackup.pstate;

        fatalThrowWithContext(0x1337, FatalPolicy_ErrorScreen, &fatalCtx);
        while (true)
            svcSleepThread(UINT64_MAX);
    }

    alreadyCrashed = true;

    printDebugInfo(ctx, base_address);

    std::memcpy(&ctxBackup, ctx, sizeof(ThreadExceptionDump));

    const char *errorDesc = nullptr;

    switch (ctx->error_desc) {
        case ThreadExceptionDesc_BadSVC:
            errorDesc = "Bad SVC";
            break;
        case ThreadExceptionDesc_InstructionAbort:
            errorDesc = "Instruction Abort";
            break;
        case ThreadExceptionDesc_MisalignedPC:
            errorDesc = "Misaligned Program Counter";
            break;
        case ThreadExceptionDesc_MisalignedSP:
            errorDesc = "Misaligned Stack Pointer";
            break;
        case ThreadExceptionDesc_SError:
            errorDesc = "SError";
            break;
        case ThreadExceptionDesc_Trap:
            errorDesc = "SIGTRAP";
            break;
        case ThreadExceptionDesc_Other:
            errorDesc = "Segmentation Fault";
            break;
        default:
            errorDesc = "Unknown Exception";
            break;
    }

    g_crashFrame = new brls::CrashFrame(fmt::MakeString("%s\n\n%s: %s\nPC: BASE + 0x%016lx",
                                                        ~album::FATAL_EXCEPTION,
                                                        ~album::REASON,
                                                        errorDesc,
                                                        ctx->pc.x - base_address));

    if (threadGetSelf()->stack_mem == nullptr) {
        /* Is UI thread */
        brls::Application::pushView(g_crashFrame);
        g_crashFrame = nullptr;
        while (brls::Application::mainLoop())
            ;

        __nx_applet_exit_mode = 1;
        exit(1);
    } else {
        threadExit();
    }
}

brls::CrashFrame *getCrashFrame() {
    auto *frame = g_crashFrame;
    g_crashFrame = nullptr;
    return frame;
}
