/* Copyright (c) 2018 MAKER.                                                  */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

/* memory.c: Physical, virtual and memory managment                           */

#include "types.h"
#include "utils.h"
#include "kernel/logger.h"
#include "kernel/memory.h"
#include "kernel/paging.h"

/* --- Private functions ---------------------------------------------------- */

uint TOTAL_MEMORY = 0;
uint USED_MEMORY = 0;

uchar MEMORY[1024 * 1024 / 8];

#define PHYSICAL_IS_USED(addr) \
    (MEMORY[(uint)addr / PAGE_SIZE / 8] & (1 << (uint)addr / PAGE_SIZE % 8))

#define PHYSICAL_SET_USED(addr) \
    MEMORY[(uint)addr / PAGE_SIZE / 8] |= (1 << (uint)addr / PAGE_SIZE % 8)

#define PHYSICAL_SET_FREE(addr) \
    MEMORY[(uint)addr / PAGE_SIZE / 8] &= ~(1 << (uint)addr / PAGE_SIZE % 8)

int physical_is_used(uint addr, uint count)
{
    for (uint i = 0; i < count; i++)
    {
        if (PHYSICAL_IS_USED(addr + i * PAGE_SIZE))
            return 1;
    }

    return 0;
}

void physical_set_used(uint addr, uint count)
{
    for (uint i = 0; i < count; i++)
    {
        PHYSICAL_SET_USED(addr + i * PAGE_SIZE);
    }
}

void physical_set_free(uint addr, uint count)
{
    for (uint i = 0; i < count; i++)
    {
        PHYSICAL_SET_FREE(addr + i * PAGE_SIZE);
    }
}

uint physical_alloc(uint count)
{
    for (uint i = 0; i < (TOTAL_MEMORY / PAGE_SIZE); i++)
    {
        uint addr = i * PAGE_SIZE;
        if (!physical_is_used(addr, count))
        {
            physical_set_used(addr, count);
            return addr;
        }
    }

    return 0;
}

void physical_free(uint addr, uint count)
{
    physical_set_free(addr, count);
}

// Page directory index
#define PD_INDEX(vaddr) (vaddr >> 22)

// Page table index
#define PT_INDEX(vaddr) (vaddr >> 12 && 0x03ff)

int virtual_absent(page_directorie_t *pdir, uint vaddr, uint count)
{
    int absent = 1;

    for (uint i = 0; i < count; i++)
    {
        uint offset = i * PAGE_SIZE;

        uint pdi = PD_INDEX(vaddr);
        uint pti = PT_INDEX(vaddr);

        page_directorie_entry_t *pde = &pdir->entries[pdi];

        if (pde->Present)
        {
            page_table_t *ptable = (page_table_t *)(pde->PageFrameNumber * PAGE_SIZE);
            page_t *p = &ptable->pages[pti];

            absent = absent && !p->Present;
        }
    }

    return absent;
}

int virtual_present(page_directorie_t *pdir, uint vaddr, uint count)
{
    int present = 1;

    for (uint i = 0; i < count; i++)
    {
        uint offset = i * PAGE_SIZE;

        uint pdi = PD_INDEX(vaddr);
        uint pti = PT_INDEX(vaddr);

        page_directorie_entry_t *pde = &pdir->entries[pdi];

        if (pde->Present)
        {
            page_table_t *ptable = (page_table_t *)(pde->PageFrameNumber * PAGE_SIZE);
            page_t *p = &ptable->pages[pti];

            present = present && p->Present;
        }
    }

    return present;
}

uint virtual2physical(page_directorie_t *pdir, uint vaddr)
{
    uint pdi = PD_INDEX(vaddr);
    uint pti = PT_INDEX(vaddr);

    page_directorie_entry_t *pde = &pdir->entries[pdi];
    page_table_t *ptable = (page_table_t *)(pde->PageFrameNumber * PAGE_SIZE);
    page_t *p = &ptable->pages[pti];

    return ((p->PageFrameNumber & ~0xfff) + (vaddr & 0xfff));
}

uint virtual_map(page_directorie_t *pdir, uint vaddr, uint count, uint paddr, bool user)
{
    for (uint i = 0; i < count; i++)
    {
        uint offset = i * PAGE_SIZE;

        uint pdi = PD_INDEX(vaddr + offset);
        uint pti = PT_INDEX(vaddr + offset);

        page_directorie_entry_t *pde = &pdir->entries[pdi];
        page_table_t *ptable = (page_table_t *)(pde->PageFrameNumber * PAGE_SIZE);
        page_t *p = &ptable->pages[pti];

        p->as_uint = 0;
        
    }

    paging_invalidate_tlb();
}

void virtual_unmap(page_directorie_t *pdir, uint vaddr, uint count)
{
    for (uint i = 0; i < count; i++)
    {
        uint offset = i * PAGE_SIZE;

        uint pdi = PD_INDEX(vaddr + offset);
        uint pti = PT_INDEX(vaddr + offset);

        page_directorie_entry_t *pde = &pdir->entries[pdi];
        page_table_t *ptable = (page_table_t *)(pde->PageFrameNumber * PAGE_SIZE);
        page_t *p = &ptable->pages[pti];

        p->as_uint = 0;
    }

    paging_invalidate_tlb();
}

/* --- Public functions ----------------------------------------------------- */

page_directorie_t ALIGNED(kpdir, PAGE_SIZE);
page_table_t ALIGNED(kptable[256], PAGE_SIZE);

void memory_setup(uint used, uint total)
{
    TOTAL_MEMORY = total;

    // Setup the kernel pagedirectorie.
    for (uint i = 0; i < 256; i++)
    {
        page_directorie_entry_t *e = &kpdir.entries[i];
        e->User = false;
        e->Write = true;
        e->Present = true;
        e->PageFrameNumber = (uint)&kptable[i] / PAGE_SIZE;
    }

    // Map the kernel memory
    uint count = PAGE_ALIGN(used) / PAGE_SIZE;

    physical_set_used(0, count);
    virtual_map(&kpdir, 0, count, 0, 0);
}

// Alloc a pdir for a process
page_directorie_t *memory_alloc_pdir()
{
}

// Free the pdir of a dying process
void memory_free_pdir(page_directorie_t *pdir)
{
}

uint memory_alloc(uint count)
{
}

void memory_free(uint addr, uint count)
{
}

int memory_map(page_directorie_t *pdir, uint addr, uint count)
{
}

int memory_unmap(page_directorie_t *pdir, uint addr, uint count)
{
}
