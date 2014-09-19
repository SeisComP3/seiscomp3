#ifndef __@LIBRARY@_API_H__
#define __@LIBRARY@_API_H__

#if defined(WIN32) && (defined(@LIBRARY@_SHARED) || defined(SC_ALL_SHARED))
# if defined(@LIBRARY@_EXPORTS)
#  define @LIBRARY@_API __declspec(dllexport)
#  define @LIBRARY@_TEMPLATE_EXPORT
# else
#  define @LIBRARY@_API __declspec(dllimport)
#  define @LIBRARY@_TEMPLATE_EXPORT extern
# endif
#else
# define @LIBRARY@_API
# define @LIBRARY@_TEMPLATE_EXPORT
#endif

#endif
