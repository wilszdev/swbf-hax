#pragma once

class PointerPath
{
public:
	PointerPath(HANDLE process, uintptr_t ptr, std::vector<unsigned int> offsets);
	uintptr_t Resolve();
	uintptr_t Ptr();
	bool Read(void* out, size_t length);
	bool Write(void* in, size_t length);
private:
	HANDLE m_process;
	uintptr_t m_ptr;
	std::vector<unsigned int> m_offsets;
	uintptr_t m_cachedAddress;
};

