#pragma once

#include "json.hpp"

#include <coreinit/dynload.h>

#include <algorithm>
#include <forward_list>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#define LIMIT(x, min, max)                                   \
    ({                                                       \
        typeof(x) _x     = x;                                \
        typeof(min) _min = min;                              \
        typeof(max) _max = max;                              \
        (((_x) < (_min)) ? (_min) : ((_x) > (_max)) ? (_max) \
                                                    : (_x)); \
    })

#define DegToRad(a)           ((a) *0.01745329252f)
#define RadToDeg(a)           ((a) *57.29577951f)

#define ALIGN4(x)             (((x) + 3) & ~3)
#define ALIGN32(x)            (((x) + 31) & ~31)

#define ALIGN_DATA(align)     __attribute__((aligned(align)))
#define ALIGN_DATA_0x40       ALIGN_DATA(0x40)

// those work only in powers of 2
#define ROUNDDOWN(val, align) ((val) & ~(align - 1))
#define ROUNDUP(val, align)   ROUNDDOWN(((val) + (align - 1)), align)


#define le16(i)               ((((uint16_t) ((i) &0xFF)) << 8) | ((uint16_t) (((i) &0xFF00) >> 8)))
#define le32(i)               ((((uint32_t) le16((i) &0xFFFF)) << 16) | ((uint32_t) le16(((i) &0xFFFF0000) >> 16)))
#define le64(i)               ((((uint64_t) le32((i) &0xFFFFFFFFLL)) << 32) | ((uint64_t) le32(((i) &0xFFFFFFFF00000000LL) >> 32)))

//Needs to have log_init() called beforehand.
void dumpHex(const void *data, size_t size);

#ifdef __cplusplus
}
#endif

template<class T, class... Args>
std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
    return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

template<typename T>
inline typename std::unique_ptr<T> make_unique_nothrow(size_t num) noexcept {
    return std::unique_ptr<T>(new (std::nothrow) std::remove_extent_t<T>[num]());
}

template<class T, class... Args>
std::shared_ptr<T> make_shared_nothrow(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
    return std::shared_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

template<typename Container, typename Predicate>
std::enable_if_t<std::is_same_v<Container, std::forward_list<typename Container::value_type>>, bool>
remove_first_if(Container &container, Predicate pred) {
    auto it = container.before_begin();

    for (auto prev = it, current = ++it; current != container.end(); ++prev, ++current) {
        if (pred(*current)) {
            container.erase_after(prev);
            return true;
        }
    }

    return false;
}

template<typename Container, typename Predicate>
std::enable_if_t<std::is_same_v<Container, std::set<typename Container::value_type, typename Container::key_compare>>, bool>
remove_first_if(Container &container, Predicate pred) {
    auto it = container.begin();
    while (it != container.end()) {
        if (pred(*it)) {
            container.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

template<typename Container, typename Predicate>
std::enable_if_t<std::is_same_v<Container, std::vector<typename Container::value_type>>, bool>
remove_first_if(Container &container, Predicate pred) {
    auto it = container.begin();
    while (it != container.end()) {
        if (pred(*it)) {
            container.erase(it);
            return true;
        }
        ++it;
    }

    return false;
}

template<typename Container, typename Predicate>
bool remove_locked_first_if(std::mutex &mutex, Container &container, Predicate pred) {
    std::lock_guard<std::mutex> lock(mutex);
    return remove_first_if(container, pred);
}

template<typename T, typename Predicate>
T pop_locked_first_if(std::mutex &mutex, std::vector<T> &container, Predicate pred) {
    std::lock_guard<std::mutex> lock(mutex);
    T result;
    auto it = container.begin();
    while (it != container.end()) {
        if (pred(*it)) {
            result = std::move(*it);
            container.erase(it);
            return result;
        }
        ++it;
    }

    return result;
}

template<typename Container>
void append_move_all_values(Container &dest, Container &src) {
    dest.insert(dest.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
    src.clear();
}

typedef enum {
    UTILS_IO_ERROR_SUCCESS       = 0,     /**< Success. */
    UTILS_IO_ERROR_INVALID_ARGS  = -0x01, /**< Invalid arguments passed to the function. */
    UTILS_IO_ERROR_MALLOC_FAILED = -0x02, /**< Memory allocation failed. */
    UTILS_IO_ERROR_GENERIC       = -0x03, /**< Generic IO error during saving or loading. */
    UTILS_IO_ERROR_NOT_FOUND     = -0x4,  /**< Item not found. */
} UtilsIOError;

std::string getPluginPath();

std::string getModulePath();

OSDynLoad_Error CustomDynLoadAlloc(int32_t size, int32_t align, void **outAddr);

void CustomDynLoadFree(void *addr);

UtilsIOError ParseJsonFromFile(const std::string &filePath, nlohmann::json &outJson);

std::vector<std::string> getPluginFilePaths(std::string_view basePath);

std::vector<std::string> getNonBaseAromaPluginFilenames(std::string_view basePath);