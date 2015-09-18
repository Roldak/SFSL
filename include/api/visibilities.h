//
//  Visibilities.h
//  SFSL
//
//  Created by Romain Beguet on 17.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Visibilities__
#define __SFSL__API_Visibilities__

// source: https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILD_API
    #ifdef __GNUC__
      #define SFSL_API_PUBLIC __attribute__ ((dllexport))
    #else
      #define SFSL_API_PUBLIC __declspec(dllexport)
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

#define PASTER(x,y) x ## y
#define EVAL_PASTER(x,y)  PASTER(x,y)

#define PRIVATE_IMPL_NAMESPACE_NAME __priv
#define PRIVATE_IMPL_SUFFIX Impl

#define DECL_PRIVATE_IMPL_FOR(type) namespace sfsl { namespace PRIVATE_IMPL_NAMESPACE_NAME { class EVAL_PASTER(type, PRIVATE_IMPL_SUFFIX); } }
#define BEGIN_PRIVATE_DEF namespace sfsl { namespace PRIVATE_IMPL_NAMESPACE_NAME {
#define END_PRIVATE_DEF } }

#define PRIVATE_IMPL(type) EVAL_PASTER(sfsl::PRIVATE_IMPL_NAMESPACE_NAME::type, PRIVATE_IMPL_SUFFIX)
#define PRIVATE_IMPL_PTR(type) PRIVATE_IMPL(type)*
#define NAME_OF_IMPL(type) EVAL_PASTER(type, PRIVATE_IMPL_SUFFIX)

#define NEW_PRIV_IMPL(type) new PRIVATE_IMPL(type)

#endif
