// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#if !defined(GENERIC_NAME) || !defined(GENERIC_PREFIX)
#error GENERIC_NAME and GENERIC_PREFIX must be defined before including a header for defining a generic class
#endif

#define _GENERIC_CONCAT(pfx, x) pfx##x
#define GENERIC_CONCAT(pfx, x) _GENERIC_CONCAT(pfx, x)

#define _GENERIC_STRINGIZE(s) #s
#define GENERIC_STRINGIZE(s) _GENERIC_STRINGIZE(s)

#define NAME GENERIC_NAME
#define NAME_STR GENERIC_STRINGIZE(NAME)

#define FUNC(name) GENERIC_CONCAT(GENERIC_PREFIX, name)
#define VAR(name) GENERIC_CONCAT(GENERIC_PREFIX, name)
#ifdef GENERIC_IMPL_STATIC
#define GENERIC_IMPL
#define FUNCDECL(ret_type, name) __attribute__((unused)) static ret_type FUNC(name)
#define FUNCDEF(ret_type, name)  __attribute__((unused))        ret_type FUNC(name)
#define VARDECL(type, name) static type VAR(name)
#define VARDEF(type, name)  static type VAR(name)
#else
#define FUNCDECL(ret_type, name) ret_type FUNC(name)
#define FUNCDEF(ret_type, name)  ret_type FUNC(name)
#define VARDECL(type, name) extern type VAR(name)
#define VARDEF(type, name)         type VAR(name)
#endif

#ifdef GENERIC_REQUIRE_TYPE
#define GENERIC_ALLOW_TYPE
#endif
#ifdef GENERIC_REQUIRE_VALUE_TYPE
#define GENERIC_ALLOW_VALUE_TYPE
#endif
#ifdef GENERIC_REQUIRE_KEY_TYPE
#define GENERIC_ALLOW_KEY_TYPE
#endif

#if defined(GENERIC_REQUIRE_TYPE) && !defined(GENERIC_TYPE)
#error Defining GENERIC_TYPE is required for defining this generic class
#endif
#if defined(GENERIC_REQUIRE_VALUE_TYPE) && !defined(GENERIC_VALUE_TYPE)
#error Defining GENERIC_VALUE_TYPE is required for defining this generic class
#endif
#if defined(GENERIC_REQUIRE_KEY_TYPE) && !defined(GENERIC_KEY_TYPE)
#error Defining GENERIC_KEY_TYPE is required for defining this generic class
#endif

#if !defined(GENERIC_ALLOW_TYPE) && defined(GENERIC_TYPE)
#error Defining GENERIC_TYPE is not allowed for defining this generic class
#endif
#if !defined(GENERIC_ALLOW_VALUE_TYPE) && defined(GENERIC_VALUE_TYPE)
#error Defining GENERIC_VALUE_TYPE is not allowed for defining this generic class
#endif
#if !defined(GENERIC_ALLOW_KEY_TYPE) && defined(GENERIC_KEY_TYPE)
#error Defining GENERIC_KEY_TYPE is not allowed for defining this generic class
#endif

#if defined(GENERIC_TYPE)
#define TYPE GENERIC_TYPE
#endif
#if defined(GENERIC_VALUE_TYPE)
#define VTYPE GENERIC_VALUE_TYPE
#endif
#if defined(GENERIC_KEY_TYPE)
#define KTYPE GENERIC_KEY_TYPE
#endif

#ifndef GENERIC_TERM_ITEM
#define GENERIC_TERM_ITEM(_itm)
#endif
