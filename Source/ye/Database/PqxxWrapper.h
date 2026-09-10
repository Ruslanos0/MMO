// Copyright Epic Games, Inc. All Rights Reserved.
// This header isolates pqxx from UE5 memory overrides

#pragma once

// Undefine UE5 memory macros before including pqxx
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4191 4996 4530)
#endif

// Temporarily restore standard new/delete
#ifdef operator new
#undef operator new
#endif
#ifdef operator delete
#undef operator delete
#endif

THIRD_PARTY_INCLUDES_START
#include <pqxx/pqxx>
THIRD_PARTY_INCLUDES_END

#ifdef _MSC_VER
#pragma warning(pop)
#endif
