# The edition name the application reports about itself. `lusan_add_application` passes it as
# the LUSAN_EDITION compile definition; nothing else in the build depends on it.
set(LUSAN_EDITION "Free" CACHE STRING "Edition name the application reports about itself")
