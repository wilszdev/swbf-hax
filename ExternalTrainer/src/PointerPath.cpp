#include "pch.h"
#include "PointerPath.h"
#include "mem.h"

PointerPath::PointerPath(HANDLE process, uintptr_t ptr, std::vector<unsigned int> offsets)
	: m_process(process), m_ptr(ptr), m_offsets(offsets), m_cachedAddress(0)
{
}

uintptr_t PointerPath::Resolve()
{
	m_cachedAddress = mem::ResolvePtrPath(m_process, m_ptr, m_offsets);
	return m_cachedAddress;
}

uintptr_t PointerPath::Ptr()
{
	return m_cachedAddress;
}

bool PointerPath::Read(void* out, size_t length)
{
	return m_cachedAddress && ReadProcessMemory(m_process, (void*)m_cachedAddress, out, length, 0);
}

bool PointerPath::Write(void* in, size_t length)
{
	return m_cachedAddress && WriteProcessMemory(m_process, (void*)m_cachedAddress, in, length, 0);
}