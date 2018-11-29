#pragma once

template<typename T>
__forceinline constexpr static T callVirtualFunction(void* ppClass, int index) noexcept
{
    int* pVTable = *(int**)ppClass;
    int dwAddress = pVTable[index];
    return (T)(dwAddress);
}
