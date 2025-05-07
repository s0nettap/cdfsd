#pragma once

#include <cstdint>

class VMT {
private:
    void** vmt;

public:
    VMT(void* class_base) {
        vmt = *reinterpret_cast<void***>(class_base);
    }

    template <typename T>
    T get_method(uint32_t index) {
        return reinterpret_cast<T>(vmt[index]);
    }

    static uintptr_t get_v_method(void* class_base, uint32_t index) {
        VMT vmt_hook(class_base);
        return reinterpret_cast<uintptr_t>(vmt_hook.get_method<void*>(index));
    }
};