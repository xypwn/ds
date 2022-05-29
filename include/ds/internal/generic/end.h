// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#if defined(GENERIC_TYPE)
#undef TYPE
#undef GENERIC_TYPE
#endif
#if defined(GENERIC_VALUE_TYPE)
#undef VTYPE
#undef GENERIC_VALUE_TYPE
#endif
#if defined(GENERIC_KEY_TYPE)
#undef KTYPE
#undef GENERIC_KEY_TYPE
#endif

#ifdef GENERIC_REQUIRE_TYPE
#undef GENERIC_REQUIRE_TYPE
#endif
#ifdef GENERIC_REQUIRE_VALUE_TYPE
#undef GENERIC_REQUIRE_VALUE_TYPE
#endif
#ifdef GENERIC_REQUIRE_KEY_TYPE
#undef GENERIC_REQUIRE_KEY_TYPE
#endif

#ifdef GENERIC_ALLOW_TYPE
#undef GENERIC_ALLOW_TYPE
#endif
#ifdef GENERIC_ALLOW_VALUE_TYPE
#undef GENERIC_ALLOW_VALUE_TYPE
#endif
#ifdef GENERIC_ALLOW_KEY_TYPE
#undef GENERIC_ALLOW_KEY_TYPE
#endif

#undef GENERIC_PREFIX
#undef FUNC
#undef FUNCDECL
#undef FUNCDEF
#undef VAR
#undef VARDECL
#undef VARDEF

#undef NAME
#undef NAME_STR
#undef GENERIC_NAME

#undef _GENERIC_STRINGIZE
#undef GENERIC_STRINGIZE

#undef _GENERIC_CONCAT
#undef GENERIC_CONCAT
