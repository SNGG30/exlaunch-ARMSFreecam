#pragma once

#include <lib.hpp>

namespace free_king::free_cam {

    namespace impl {
        // static constexpr uintptr_t LpLyr2D_OffVtable = 0x02CB2558;
        // static constexpr uintptr_t LpLyr3D_OffVtable = 0x02CB3550;

        static constexpr uintptr_t SystemTask_FuncCalcFrame = 0x00A6F32C;

        static constexpr uintptr_t Renderer_OffDbgState = 0x558;
        static constexpr uintptr_t Renderer_OffController = 0x560;

        static constexpr uintptr_t Layer_FuncCalc = 0x009BCD48;
        static constexpr uintptr_t Layer_OffDbgInfo = 0x1F0;

        // static constexpr uintptr_t DevTools_OffsCameraOperationSpeed = 0x02C99B10;

        // static constexpr uintptr_t DbgInfo_OffStrs = 0x3A8;

        static constexpr uintptr_t ControllerMgr_OffsInstance = 0x0174DE18;
        static constexpr uintptr_t ControllerMgr_OffControllerArray = 0x188;
    }

    void Install();

    // static inline auto& GetCamOpSpeed() {
    //     return *exl::util::pointer_path::FollowSafe<float, impl::DevTools_OffsCameraOperationSpeed>();
    // }
}