/****************************************************************
 *								*
 * Copyright (c) 2001-2016 Fidelity National Information	*
 * Services, Inc. and/or its subsidiaries. All rights reserved.	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#ifdef __CYGWIN__
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A CYGWIN x86"
#elif defined(__ia64)
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A Linux IA64"
#elif defined(__x86_64__)
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A Linux x86_64"
#elif defined(__s390__)
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A Linux S390X"
#elif defined(__armv7l__)
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A Linux armv7l"
#else
#define GTM_RELEASE_NAME 	"GT.M V6.3-000A Linux x86"
#endif
#define GTM_PRODUCT 		"GT.M"
#define GTM_VERSION		"V6.3"
