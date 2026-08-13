#include "allocations_checker.h"

#include <atomic>
#include <new>
#include <stdexcept>

#include <stdlib.h>

#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#define HAS_SANITIZER
#include <sanitizer/allocator_interface.h>
#endif
#endif

std::atomic<size_t> allocations_count{0}, deallocations_count{0};

namespace alloc_checker {

size_t allocCount() {
	return allocations_count.load();
}

size_t deallocCount() {
	return deallocations_count.load();
}

void resetCounters() {
	allocations_count.store(0);
	deallocations_count.store(0);
}

} // namespace alloc_checker

void MallocHook(void const volatile *, size_t) {
	allocations_count.fetch_add(1);
}

void FreeHook(void const volatile *) {
	deallocations_count.fetch_add(1);
}

#ifdef HAS_SANITIZER
[[maybe_unused]] const auto kInit = [] {
	int res = __sanitizer_install_malloc_and_free_hooks(MallocHook, FreeHook);
	if(res == 0) {
		throw std::runtime_error{
		        "Failed to install ASan allocator hooks"}; // just terminate
	}
	return 0;
}();
#else
void * operator new(size_t size) {
	void * p = malloc(size);
	MallocHook(p, size);
	return p;
}

void * operator new(size_t size, std::nothrow_t const &) noexcept {
	void * p = malloc(size);
	MallocHook(p, size);
	return p;
}

void * operator new[](size_t size) {
	void * p = malloc(size);
	MallocHook(p, size);
	return p;
}

void * operator new[](size_t size, std::nothrow_t const &) noexcept {
	void * p = malloc(size);
	MallocHook(p, size);
	return p;
}

void operator delete(void * p) noexcept {
	FreeHook(p);
	free(p);
}

void operator delete(void * p, size_t) noexcept {
	FreeHook(p);
	free(p);
}

void operator delete[](void * p) noexcept {
	FreeHook(p);
	free(p);
}

void operator delete[](void * p, size_t) noexcept {
	FreeHook(p);
	free(p);
}
#endif
