#ifndef __SFSL__API_Visibilities__
#define __SFSL__API_Visibilities__

// source: https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILD_DLL
    #ifdef __GNUC__
      #define SFSL_API_PUBLIC __attribute__ ((dllexport))
    #else
      #define SFSL_API_PUBLIC declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define SFSL_API_PUBLIC __attribute__ ((dllimport))
    #else
      #define SFSL_API_PUBLIC __declspec(dllimport)
    #endif
  #endif
  #define SFSL_API_LOCAL
#else
  #if __GNUC__ >= 4
    #define SFSL_API_PUBLIC __attribute__ ((visibility ("default")))
    #define SFSL_API_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SFSL_API_PUBLIC
    #define SFSL_API_LOCAL
  #endif
#endif

#endif