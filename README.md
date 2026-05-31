# UEFI Forth

A Forth interpreter running on UEFI, built around an event-driven cooperative multitasking architecture.

## Overview

This is a native UEFI application that implements a Forth interpreter with the following characteristics:

- **Event-driven loop** — Uses UEFI's `WaitForEvent`. All I/O (keyboard input, timers) is handled through UEFI events and callbacks. When no events are pending, the CPU blocks inside `WaitForEvent` with no busy-wait — this is low-power friendly by design, as the firmware can place the CPU into an idle state (e.g. C-states on x86) until an event fires.
- **Cooperative multitasking** — Multiple Forth tasks run in a circular linked list. Tasks yield when waiting for I/O and resume when events fire.
- **Coroutine yield/resume** — GCC computed goto (`&&label` / `goto *save`) implements lightweight coroutines inside the inner interpreter. a yielded task simply returns from `forth_run` and re-enters at the saved label on the next scheduler tick.
- **Single dictionary, shared across tasks** — All tasks share one 8MB dictionary space. Compilation is single-threaded by design.

### Task scheduling

Each `forth_context` holds its own parameter stack, return stack, and instruction pointer. Tasks form a circular linked list. `forth_run_all()` iterates the list once per tick, running each task until it yields (I/O wait) or halts (error/shutdown).

## Forth model

- **Cell size**: `uintptr_t` (8 bytes on x86_64 UEFI)
- **Stack depth**: 128 cells per task
- **Character encoding**: CHAR16 (UTF-16), matching UEFI conventions
- **Number format**: Hex only, `$` prefix (e.g. `$1A`, `$FF`)
- **Dictionary**: threaded code — each word body is a sequence of XT pointers

## Build

Requires clang with x86_64 UEFI target support, GNU-EFI headers, and `genimage` for disk image creation.

```bash
make            # build BOOTX64.EFI
make image      # create disk image
make qemu       # run in QEMU with OVMF
```

### Dependencies

- clang (x86_64-pc-win32-coff target)
- lld-link
- GNU-EFI (included as submodule)
- OVMF firmware (`/usr/share/ovmf/x64/OVMF.4m.fd`)
- genimage
- QEMU (for testing)

## Limitations

- GCC-specific computed goto for coroutine yield/resume
- No task deletion
- Global compile stack only one task should compile at a time

## License

MIT