/**\file n_enum.h
 * Macro to build enums and their tostring counterparts, a reduced version of https://github.com/FredBienvenu/SmartEnum
 *\author Castagnier Mickael, based on original FredBienvenu SmartEnum entry
 *\version 1.0
 *\date 28/09/2021
 */

#ifndef __N_ENUM_HEADER__
#define __N_ENUM_HEADER__

#include <string.h> // for strcmp
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**\defgroup ENUMS ENUMERATIONS: quick and clean enum macro set
   \addtogroup ENUMS
  @{
*/

/* MACROS FOR N_ENUMS MACRO ENTRIES (START) */

/*! helper to build an N_ENUM */
#define N_ENUM_ENTRY(class, method) class##_##method

/*!	utils macro case value */
#define __N_ENUM_MACRO_ENTRY_CASE(element_name) case element_name:
/*!	utils macros string compare */
#define __N_ENUM_MACRO_ENTRY_COMPARE(element_name) if(!strcmp(#element_name, str_value))
/*!	utils macros string ex compare */
#define __N_ENUM_MACRO_ENTRY_COMPARE_EX(element_name) if(!strcmp(#element_name + index, str_value))
/*!	utils macros unknown value */
#define __N_ENUM_MACRO_ENTRY_UNKNOWN_VALUE(enum_name) __##enum_name##_UNKNOWN_VALUE__


/*! macro to convert N_ENUMS entry to a real ENUM element */
#define __N_ENUM_MACRO_ENTRY_TO_ENUM_ELEMENT(element_name, element_value)\
	element_name = element_value,
/*! macro to test N_ENUMS entry */
#define __N_ENUM_MACRO_ENTRY_TO_ISVALID_CASE(element_name, element_value)\
	__N_ENUM_MACRO_ENTRY_CASE(element_name) return true;
/*! macro to convert N_ENUMS entry to a string element */
#define __N_ENUM_MACRO_ENTRY_TO_TOSTRING_CASE(element_name, element_value)\
	__N_ENUM_MACRO_ENTRY_CASE(element_name) return #element_name;
/*! macro to convert string to N_ENUMS value*/
#define __N_ENUM_MACRO_ENTRY_TO_FROMSTRING_COMPARE(element_name, element_value)\
	 __N_ENUM_MACRO_ENTRY_COMPARE(element_name) return element_name;
/*! macro to convert string to N_ENUMS value*/
#define __N_ENUM_MACRO_ENTRY_TO_FROMSTRING_COMPARE_EX(element_name, element_value)\
	 __N_ENUM_MACRO_ENTRY_COMPARE_EX(element_name) return element_name;
/*! macro to test if the string is a valid enum entry */
#define __N_ENUM_MACRO_ENTRY_TO_ISSTRINGVALID_COMPARE(element_name, element_value)\
    __N_ENUM_MACRO_ENTRY_COMPARE(element_name) return true;
/* MACROS FOR N_ENUMS MACRO ENTRIES (END) */

/* MACRO FOR ENUM DECLARATION (START) */
/*! macro for ENUM declaration */
#define __N_ENUM_DECLARE_ENUM(MACRO_DEFINITION, enum_name)\
	typedef enum enum_name\
{\
	MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_ENUM_ELEMENT)\
	__N_ENUM_MACRO_ENTRY_UNKNOWN_VALUE(enum_name)\
}\
enum_name;
/* MACRO FOR ENUM DECLARATION (END) */

/* MACRO FOR N_ENUMS FUNCTIONS * PREFIX : __N_ENUM_FUNCTION_ */
/*! create a getter function */
#define __N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION, enum_name) __N_ENUM_FUNCTION(enum_name); // MACRO for function declaration (just add ; after function)
/*! create a test function */
#define __N_ENUM_FUNCTION_ISVALID(enum_name)\
	bool N_ENUM_ENTRY(enum_name, isValid)(enum_name value)
/*! create a to string function */
#define __N_ENUM_FUNCTION_TOSTRING(enum_name)\
	const char* N_ENUM_ENTRY(enum_name, toString)(enum_name value)
/*! create a is string valid function */
#define __N_ENUM_FUNCTION_ISSTRINGVALID(enum_name)\
	bool N_ENUM_ENTRY(enum_name, isStringValid)(const char* str_value)
/*! create a value from string function */
#define __N_ENUM_FUNCTION_FROMSTRING(enum_name)\
	enum_name N_ENUM_ENTRY(enum_name, fromString)(const char* str_value)
/*! create a value from string function */
#define __N_ENUM_FUNCTION_FROMSTRING_EX(enum_name)\
	enum_name N_ENUM_ENTRY(enum_name, fromStringEx)(const char* str_value, int index)
/* MACRO FOR N_ENUMS FUNCTIONS (END) */

/* MACRO FOR N_ENUMS FUNCTIONS DEFINITIONS * PREFIX : __N_ENUM_DEFINE_FUNCTION_ */
/*! create getter functions is_valid */
#define __N_ENUM_DEFINE_FUNCTION_ISVALID(MACRO_DEFINITION, enum_name)\
	__N_ENUM_FUNCTION_ISVALID(enum_name)\
{\
	switch(value)\
	{\
		MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_ISVALID_CASE)\
		default: return false;\
	}\
}
/*! create getter functions toString */
#define __N_ENUM_DEFINE_FUNCTION_TOSTRING(MACRO_DEFINITION, enum_name)\
	__N_ENUM_FUNCTION_TOSTRING(enum_name)\
{\
	switch(value)\
	{\
		MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_TOSTRING_CASE)\
		default: return 0;\
	}\
}
/*! create getter for function isStringValue */
#define __N_ENUM_DEFINE_FUNCTION_ISSTRINGVALID(MACRO_DEFINITION, enum_name)\
	__N_ENUM_FUNCTION_ISSTRINGVALID(enum_name)\
{\
	MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_ISSTRINGVALID_COMPARE)\
	return false;\
}
/*! create getter functions fromString */
#define __N_ENUM_DEFINE_FUNCTION_FROMSTRING(MACRO_DEFINITION, enum_name)\
	__N_ENUM_FUNCTION_FROMSTRING(enum_name)\
{\
	MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_FROMSTRING_COMPARE)\
	return __N_ENUM_MACRO_ENTRY_UNKNOWN_VALUE(enum_name);\
}
/*! create getter functions fromStringEx */
#define __N_ENUM_DEFINE_FUNCTION_FROMSTRING_EX(MACRO_DEFINITION, enum_name)\
	__N_ENUM_FUNCTION_FROMSTRING_EX(enum_name)\
{\
	MACRO_DEFINITION(__N_ENUM_MACRO_ENTRY_TO_FROMSTRING_COMPARE_EX)\
	return __N_ENUM_MACRO_ENTRY_UNKNOWN_VALUE(enum_name);\
}
/* MACRO FOR N_ENUMS FUNCTIONS DEFINITIONS (END) */

/* N_ENUM DECLARATION */
/*! Macro to declare a N_ENUM */
#define N_ENUM_DECLARE(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DECLARE_ENUM(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION_ISVALID, enum_name)\
	__N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION_ISSTRINGVALID, enum_name)\
	__N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION_TOSTRING, enum_name)\
	__N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION_FROMSTRING, enum_name)\
	__N_ENUM_DECLARE_FUNCTION(__N_ENUM_FUNCTION_FROMSTRING_EX, enum_name)
/* N_ENUM DECLARATION (END) */

/* N_ENUM DEFINITION */
/*! Macro to define an N_ENUM */
#define N_ENUM_DEFINE(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DEFINE_FUNCTION_ISVALID(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DEFINE_FUNCTION_ISSTRINGVALID(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DEFINE_FUNCTION_TOSTRING(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DEFINE_FUNCTION_FROMSTRING(MACRO_DEFINITION, enum_name)\
	__N_ENUM_DEFINE_FUNCTION_FROMSTRING_EX(MACRO_DEFINITION, enum_name)
/* N_ENUM DEFINITION (END) */

/**
@}
*/

#ifdef __cplusplus
}
#endif

#endif // header guard
