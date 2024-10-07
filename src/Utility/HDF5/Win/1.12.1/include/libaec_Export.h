
#ifndef libaec_EXPORT_H
#define libaec_EXPORT_H

#ifdef libaec_BUILT_AS_STATIC
#  define libaec_EXPORT
#  define LIBAEC_NO_EXPORT
#else
#  ifndef libaec_EXPORT
#    ifdef aec_static_EXPORTS
        /* We are building this library */
#      define libaec_EXPORT 
#    else
        /* We are using this library */
#      define libaec_EXPORT 
#    endif
#  endif

#  ifndef LIBAEC_NO_EXPORT
#    define LIBAEC_NO_EXPORT 
#  endif
#endif

#ifndef LIBAEC_DEPRECATED
#  define LIBAEC_DEPRECATED __declspec(deprecated)
#endif

#ifndef LIBAEC_DEPRECATED_EXPORT
#  define LIBAEC_DEPRECATED_EXPORT libaec_EXPORT LIBAEC_DEPRECATED
#endif

#ifndef LIBAEC_DEPRECATED_NO_EXPORT
#  define LIBAEC_DEPRECATED_NO_EXPORT LIBAEC_NO_EXPORT LIBAEC_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBAEC_NO_DEPRECATED
#    define LIBAEC_NO_DEPRECATED
#  endif
#endif

#endif /* libaec_EXPORT_H */
