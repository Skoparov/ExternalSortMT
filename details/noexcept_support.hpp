// VS2013 compiler does not support noexcept
// so i had to come up with this workaround

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif
