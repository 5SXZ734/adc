#pragma once

struct IMAGE_FILE_HEADER_EX
{
	DWORD	NumberOfSections;
};

#define IMAGE_FILE_MACHINE_ARM64	777

extern GUID	EXTENDED_COFF_OBJ_GUID;