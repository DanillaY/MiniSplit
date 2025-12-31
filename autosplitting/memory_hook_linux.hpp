#include <cstdint>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

const int JMP_REL32_SIZE = 5;

struct Hook {
    uintptr_t target;
    uintptr_t trampoline;
    size_t stolen_size;
};

//has to be loaded with the executable to be able to read the specified function pointer
Hook hook_function_32(
    uintptr_t target_function,
    uintptr_t hook_entry,
    size_t replace_byte_size
) {
    //instructions to replace has to be at least 5 bytes long to fit the jump instruction
    if (replace_byte_size < JMP_REL32_SIZE) {
        return {};
    }

    uintptr_t page = target_function & ~(getpagesize() - 1);
    mprotect((void*)page, getpagesize(),
             PROT_READ | PROT_WRITE | PROT_EXEC);

    uint8_t* tramp = (uint8_t*)mmap(
        nullptr,
        replace_byte_size + JMP_REL32_SIZE,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    memcpy(tramp, (void*)target_function, replace_byte_size);

    tramp[replace_byte_size] = 0xE9;
    *(uint32_t*)(tramp + replace_byte_size + 1) =
        (target_function + replace_byte_size) -
        ((uintptr_t)tramp + replace_byte_size + JMP_REL32_SIZE);

    //patch with jmp hook
    *(uint8_t*)target_function = 0xE9;
    *(uint32_t*)(target_function + 1) =
        hook_entry - (target_function + JMP_REL32_SIZE);

    //fill the rest with NOP
    for (size_t i = JMP_REL32_SIZE; i < replace_byte_size; i++) {
        *(uint8_t*)(target_function + i) = 0x90;
    }

    return { target_function, (uintptr_t)tramp, replace_byte_size };
}