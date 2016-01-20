

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Jan 20 13:45:27 2016
 */
/* Compiler settings for FBShell.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __FBShell_h_h__
#define __FBShell_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IconExtractor_FWD_DEFINED__
#define __IconExtractor_FWD_DEFINED__

#ifdef __cplusplus
typedef class IconExtractor IconExtractor;
#else
typedef struct IconExtractor IconExtractor;
#endif /* __cplusplus */

#endif 	/* __IconExtractor_FWD_DEFINED__ */


#ifndef __ContextMenu_FWD_DEFINED__
#define __ContextMenu_FWD_DEFINED__

#ifdef __cplusplus
typedef class ContextMenu ContextMenu;
#else
typedef struct ContextMenu ContextMenu;
#endif /* __cplusplus */

#endif 	/* __ContextMenu_FWD_DEFINED__ */


#ifndef __ColumnProvider_FWD_DEFINED__
#define __ColumnProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class ColumnProvider ColumnProvider;
#else
typedef struct ColumnProvider ColumnProvider;
#endif /* __cplusplus */

#endif 	/* __ColumnProvider_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "shobjIdl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __FBShellLib_LIBRARY_DEFINED__
#define __FBShellLib_LIBRARY_DEFINED__

/* library FBShellLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_FBShellLib;

EXTERN_C const CLSID CLSID_IconExtractor;

#ifdef __cplusplus

class DECLSPEC_UUID("E4D8441D-F89C-4b5c-90AC-A857E1768F1F")
IconExtractor;
#endif

EXTERN_C const CLSID CLSID_ContextMenu;

#ifdef __cplusplus

class DECLSPEC_UUID("FDABCF3B-57BE-4110-94B5-4EF8EE3C6A62")
ContextMenu;
#endif

EXTERN_C const CLSID CLSID_ColumnProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("8CBB373E-693A-4bea-ADF3-D05EAE41684B")
ColumnProvider;
#endif
#endif /* __FBShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


