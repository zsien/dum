include(FetchContent)

option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." OFF)
option(FIND_LIBUV "Try finding libuv library development files in the system" ON)

FetchContent_Declare(uvw
    GIT_REPOSITORY https://github.com/skypjack/uvw.git
    GIT_TAG v3.1.0_libuv_v1.45
)

FetchContent_MakeAvailable(uvw)
