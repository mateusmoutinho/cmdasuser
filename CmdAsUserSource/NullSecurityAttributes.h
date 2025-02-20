//////////////////////////////////////////////////////////////////////////////
// NullSecurityAttributes.h : Manage security descriptors.
//////////////////////////////////////////////////////////////////////////////
#ifndef NullSecurityAttributes_Included
#define NullSecurityAttributes_Included

#include <windows.h>
#include <tchar.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////////////
// Create security attributes to allow EVERYONE access (NULL dacl).
//////////////////////////////////////////////////////////////////////////////
typedef struct tagNullSecurityAttributes : SECURITY_ATTRIBUTES
{
  tagNullSecurityAttributes()
  {    
    ::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    nLength = sizeof(SECURITY_ATTRIBUTES);
    lpSecurityDescriptor = &sd;
    bInheritHandle = FALSE;
  }

  SECURITY_DESCRIPTOR sd;

} NullSecurityAttributes;

typedef NullSecurityAttributes NullSA;

#endif //NullSecurityAttributes_Included
