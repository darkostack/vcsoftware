#ifndef CORE_CODE_UTILS_HPP
#define CORE_CODE_UTILS_HPP

#include <stdbool.h>

#define mtARRAY_LENGTH(aArray) (sizeof(aArray) / sizeof(aArray[0]))

#define mtARRAY_END(aArray) (&aArray[mtARRAY_LENGTH(aArray)])

#define mtALIGNED_VAR_SIZE(aSize, aAlignType) (((aSize) + (sizeof(aAlignType) - 1)) / sizeof(aAlignType))

#define mtDEFINE_ALIGNED_VAR(aName, aSize, aAlignType) \
    aAlignType aName[(((aSize) + (sizeof(aAlignType) - 1)) / sizeof(aAlignType))]

#define mtCONTAINER_OF(ptr, type, member) ({                  \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define SuccessOrExit(aStatus) \
    do                         \
    {                          \
        if ((aStatus) != 0)    \
        {                      \
            goto exit;         \
        }                      \
    } while (false)

#define VerifyOrExit(aCondition, ...) \
    do                                \
    {                                 \
        if (!(aCondition))            \
        {                             \
            __VA_ARGS__;              \
            goto exit;                \
        }                             \
    } while (false)

#define ExitNow(...) \
    do               \
    {                \
        __VA_ARGS__; \
        goto exit;   \
    } while (false)

#define IgnoreReturnValue(aStatement) \
    do                                \
    {                                 \
        if (aStatement)               \
        {                             \
        }                             \
    } while (false)

#endif /* CORE_CODE_UTILS_HPP */
