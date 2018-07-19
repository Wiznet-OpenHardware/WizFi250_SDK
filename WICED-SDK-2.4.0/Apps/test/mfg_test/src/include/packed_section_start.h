/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * Declare directives for structure packing. No padding will be provided
 * between the members of packed structures, and therefore, there is no
 * guarantee that structure members will be aligned.
 *
 * Declaring packed structures is compiler specific. In order to handle all
 * cases, packed structures should be delared as:
 *
 * #include <packed_section_start.h>
 *
 * typedef BWL_PRE_PACKED_STRUCT struct foobar_t {
 *    some_struct_members;
 * } BWL_POST_PACKED_STRUCT foobar_t;
 *
 * #include <packed_section_end.h>
 *
 */


/* Error check - BWL_PACKED_SECTION is defined in packed_section_start.h
 * and undefined in packed_section_end.h. If it is already defined at this
 * point, then there is a missing include of packed_section_end.h.
 */
#ifdef BWL_PACKED_SECTION
	#error "BWL_PACKED_SECTION is already defined!"
#else
	#define BWL_PACKED_SECTION
#endif


#if defined(_MSC_VER)
	/* Disable compiler warning about pragma pack changing alignment. */
	#pragma warning(disable:4103)

	/* The Microsoft compiler uses pragmas for structure packing. Other
	 * compilers use structure attribute modifiers. Refer to
	 * BWL_PRE_PACKED_STRUCT and BWL_POST_PACKED_STRUCT defined below.
	 */
	#if defined(BWL_DEFAULT_PACKING)
		/* Default structure packing */
		#pragma pack(push, 8)
	#else   /* BWL_PACKED_SECTION */
		#pragma pack(1)
	#endif   /* BWL_PACKED_SECTION */
#endif   /* _MSC_VER */


/* Declare compiler-specific directives for structure packing. */
#if defined(_MSC_VER)
	#define	BWL_PRE_PACKED_STRUCT
	#define	BWL_POST_PACKED_STRUCT
#elif defined(__GNUC__)
	#define	BWL_PRE_PACKED_STRUCT
	#define	BWL_POST_PACKED_STRUCT	__attribute__ ((packed))
#elif defined(__CC_ARM)
	#define	BWL_PRE_PACKED_STRUCT	__packed
	#define	BWL_POST_PACKED_STRUCT
#else
	#error "Unknown compiler!"
#endif
