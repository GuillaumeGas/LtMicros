#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>

/// @file

/// @defgroup HandleManager Handle manager group
/// @{

/// @brief This module is used to manager the kernel handles

#define INVALID_HANDLE_VALUE 0

typedef unsigned int Handle;

/// @brief Describes the handle types
enum HandleType
{
    PROCESS_HANDLE,
    HANDLE_TYPE_MAX
};

/// @brief Class used to manager kernel handles
class HandleManager
{
public:
    /// @brief Initializes the kernel handle manager
    void Init();

    /// @brief Finds or create if not exist a handle of a specific type given an object address
    /// @param[in]  type The handle type that must be created
    /// @param[in]  object A pointer to the object that must be referenced by the new handle
    /// @param[out] outHandle Pointer that will hold the new handle value
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus FindOrCreate(const HandleType type, void * object, Handle * const outHandle);

    /// @brief Finds a handle in a specific handles list and from a object ptr
    /// @param[in]  type The handle type we are looking for
    /// @param[in]  object A pointer to the object
    /// @param[out] outHandle Pointer that will hold the found handle
    /// @return STATUS_SUCCESS on success, STATUS_NOT_FOUND if the handle is not found, an error code otherwise
    KeStatus FindFromObjectPtr(const HandleType type, void * object, Handle * const outHandle);

    /// @brief Creates a handle from an object (if a handle already exist, returns an error)
    /// @param[in] type The object type
    /// @param[in] object A pointer to the object
    /// @param[out] outHandle Pointer that will hold the new handle
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateHandle(const HandleType type, void * object, Handle * const outHandle);

private:
    List * _processHandleList;

    /// @brief Does the same work than CreateHandle() but without checking if the handle already exists or not
    KeStatus _CreateHandle(const HandleType type, void * object, Handle * const outHandle);
};

#ifdef __HANDLE_MANAGER__
HandleManager gHandleManager;
#else
extern HandleManager gHandleManager;
#endif

/// @}