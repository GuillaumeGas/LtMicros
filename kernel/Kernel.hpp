#pragma once

#include <kernel/lib/Types.hpp>

#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Process.hpp>

#include "multiboot.hpp"

#define KERNEL_PAGE_DIR_P_ADDR           0x1000     // Kernel page directory address
#define KERNEL_PAGES_TABLE_P_ADDR        0x400000   // Kernel page table address
#define KERNEL_LIMIT_P_ADDR              0x800000   // Kernel limit address (more details about kernel physical memory organisation in Vmm.hpp)
#define KERNEL_STACK_P_ADDR              0xA0000    // Kernel stack address
#define KERNEL_PAGE_POOL_V_BASE_ADDR     0x800000   // Kernel page pool area base virtual address
#define KERNEL_PAGE_POOL_V_LIMIT_ADDR    0x1000000  // Kernel page pool area limit virtual address
#define KERNEL_HEAP_V_BASE_ADDR          0x1000000  // Kernel heap base virtual address
#define KERNEL_HEAP_V_LIMIT_ADDR         0x40000000 // Kernel heap limit virtual address
#define KERNEL_IMAGE_NAME                "ltkernel.img" // Kernel image name

struct KernelInfo
{
    /// @brief Kernel page directory address
	PageDirectory pPageDirectory;
    /// @brief Kernel page table address
	PageTableEntry * pPageTables;
    /// @brief Kernel limit address
	u32 pKernelLimit;
    /// @brief Kernel stack address
	u32 pStackAddr;
    /// @brief Kernel page pool area base virtual address
    u32 vPagePoolBase;
    /// @brief Kernel page pool area limit virtual address
    u32 vPagePoolLimit;
    /// @brief Kernel heap base virtual address
    u32 vHeapBase;
    /// @brief Kernel heap limit virtual address
    u32 vHeapLimit;
    /// @brief True if kernel is in debug mode (must be connected to LtDbg
	bool debug;
    /// @brief Kernel image name
    char imageName[512];

    MultibootPartialInfo * multibootInfo;
};

class Kernel
{
public:
    /// @brief Kernel class constructor, initializes the KernelInfo structure.
    Kernel();

    /// @brief Initializes kernel
    /// @param[in] mid A pointer on a multiboot partial info used to retrieve RAM information.
    /// @param[in] multibootMagicNumber An unsigned integer used to check if the kernel has been loaded correctly by GRUB
    void Init(MultibootPartialInfo * mbi, u32 multibootMagicNumber);

    /// @brief Starts LtMicros
    void Start();

    void LoadModules();

    /// @brief Gathers information about kernel
    KernelInfo info;

    /// @brief Pointer to the system process
    Process * process;

    void Panic();

private:
    ///@brief Checks multiboot info and magic number
    ///       https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html#Boot-information-format
    void CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber);

    void PrintHello();
};

#ifdef __KERNEL__
Kernel gKernel;
#else
extern Kernel gKernel;
#endif