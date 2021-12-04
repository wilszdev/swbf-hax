#pragma once

void Patch(void* dst, void* src, size_t length);

bool Hook(void* src, void* dst, size_t length);

void* TrampolineHook(void* src, void* dst, size_t length);