#include "free_cam.hpp" 

#include <sead/gfx/seadColor.h>
#include <sead/prim/seadSafeString.hpp>
#include <sead/container/seadPtrArray.h>
#include <sead/container/seadSafeArray.h>
#include <sead/controller/seadController.h>
#include <sead/gfx/seadCamera.h>
// #include "draw.hpp"
#include "misc_types.hpp"

namespace nn::hid {

    struct ControllerSupportResultInfo
    {
        int mConnectedControllers;
        int mSelectedId;
    };

    struct ControllerSupportArg
    {
        static constexpr size_t ControllerNameSize = 128 + 1;

        u8 mMinPlayerCount;
        u8 mMaxPlayerCount;
        bool mTakeOverConnection;
        bool mLeftJustify;
        bool mPermitJoyconDual;
        bool mSingleMode;
        bool mUseColors;
        sead::Color4u8 ControllerSupportArg_x7[4];
        bool mUsingControllerNames;
        char mControllerNames[4][ControllerNameSize];
    };

    Result ShowControllerSupport(ControllerSupportArg const&);
};

namespace free_king::free_cam {

    /* Control bitfields. */
    static constexpr uint FreeCamEnableBits = 0x200;        /* Minus */
    static constexpr uint ShowDbgInfoEnableBits = 0x400;    /* Plus */
    static constexpr uint FollowPlayerEnableBits = 0x10000; /* Dpad up */
    static constexpr uint CamSpeedDownBits = 0x20000;       /* Dpad down */

    using DbgInfoStrs = sead::SafeArray<sead::FixedSafeString<512>, 2>;

    #define PTR_AT(type, base, offset)    \
        reinterpret_cast<type>(reinterpret_cast<uintptr_t>(base) + offset)

    /* Quick getters/utilities. */
    static inline auto GetCtrls() {
        return exl::util::pointer_path::FollowSafe<sead::FixedPtrArray<sead::Controller, 8>, impl::ControllerMgr_OffsInstance, impl::ControllerMgr_OffControllerArray>();
    }
    static inline auto GetDbgInfo(uintptr_t layer) {
        return exl::util::pointer_path::Follow<agl::lyr::Layer::DebugInfo*, impl::Layer_OffDbgInfo>(layer);
    }
    // static inline auto GetDbgInfoStrs(uintptr_t layer) {
    //     return exl::util::pointer_path::Follow<DbgInfoStrs*, impl::DbgInfo_OffStrs>(GetDbgInfo(layer));
    // }

    /* Hacky type comparison by vtable. */
    // static bool IsLayerLp3D(uintptr_t layer) {
    //     uintptr_t vtbl = exl::util::modules::GetTargetOffset(impl::LpLyr3D_OffVtable);
    //     VTBL_WRAP(layerw, (uintptr_t*) layer);
    //     return layerw->m_Vtbl == vtbl;
    // }
    // static bool IsLayerLp2D(uintptr_t layer) {
    //     uintptr_t vtbl = exl::util::modules::GetTargetOffset(impl::LpLyr2D_OffVtable);
    //     VTBL_WRAP(layerw, (uintptr_t*) layer);
    //     return layerw->m_Vtbl == vtbl;
    // }

    static inline auto GetRenderer() {
        return exl::util::pointer_path::Follow<uintptr_t, 0x017502B8>();
    }
    
    /* Static state. */
    static bool s_FreeCamEnabled = false;
    static bool s_ShowDebugInfo = false;
    // static DbgInfoStrs* s_DebugInfoStrs = NULL;
    //static sead::LookAtCamera* s_DbgCamera;
    static bool s_FollowPlayer = false;

    HOOK_DEFINE_TRAMPOLINE(SystemTaskCalcFrame) {
        static void Callback(uintptr_t _this) {
             /* Set the debug camera state. */
            int* dbgState = PTR_AT(int*, GetRenderer(), impl::Renderer_OffDbgState);
            *dbgState = s_FreeCamEnabled;

            /* Try getting ctrl ptr array. */
            auto ctrls = GetCtrls();
            if(ctrls != NULL) {
                auto camCtrl = ctrls->at(1);

                sead::Controller** rCtrl = PTR_AT(sead::Controller**, GetRenderer(), impl::Renderer_OffController);
                *rCtrl = camCtrl;

                /* Add extra controls if we have a controller to use. */
                if(camCtrl != NULL) {
                    uint trig = camCtrl->getTrigMask();
                    // auto& camSpeed = GetCamOpSpeed();

                    // if(trig & CamSpeedUpBits) {
                    //     camSpeed += 0.5;
                    // } else if(trig & CamSpeedDownBits) {
                    //     camSpeed -= 0.5;
                    // }

                    /* Check toggling following the player. */
                    if(trig & FollowPlayerEnableBits) {
                        s_FollowPlayer = !s_FollowPlayer;
                    }

                    /* Check toggling the free cam. */
                    if(trig & FreeCamEnableBits) {
                        svcOutputDebugString("Test!", sizeof("Test!"));
                        s_FreeCamEnabled = !s_FreeCamEnabled;
                    }

                    /* Check for showing debug info. */
                    if(trig & ShowDbgInfoEnableBits) {
                        s_ShowDebugInfo = !s_ShowDebugInfo;
                    }
                }
            }

            Orig(_this);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(LayerCalc) {
        static void Callback(uintptr_t _this, uintptr_t controller, int camCtrlType, bool useTwist) {
            static constexpr u16 Bf1EnableBits = 8 | 1;
            static constexpr u16 Bf1EnableMask = ~(0x100 | 0x10);
            static constexpr u16 Bf3EnableBits = 8;

            /* Get pointers to relevant bitfields. */
            auto bf1 = PTR_AT(u16*, _this, 0x82);
            auto bf2 = PTR_AT(u16*, _this, 0x84);
            auto bf3 = PTR_AT(u16*, _this, 0x80);

            /* Write to bitfields. */
            if (s_FreeCamEnabled) {
                *bf1 |= Bf1EnableBits;
                *bf1 &= Bf1EnableMask;
                *bf3 |= Bf3EnableBits;
                *bf2 |= 2;
            }
            else {
                *bf1 &= ~Bf1EnableBits;
                *bf3 &= ~Bf3EnableBits;
                *bf2 &= ~2;
            }
            auto dbgInfo = GetDbgInfo(_this);
            dbgInfo->mFar = 100000.0f;

            // s_DebugInfoStrs = GetDbgInfoStrs(_this);

            
            /* Explicitly enable camera twist and use cam type 0. */
            Orig(_this, controller, 0, true);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(ShowControllerSupport) {
        static Result Callback(nn::hid::ControllerSupportArg const& arg) {
            /* Copy arg and modify it. */
            nn::hid::ControllerSupportArg narg = arg;

            strncpy(narg.mControllerNames[0], "Player", nn::hid::ControllerSupportArg::ControllerNameSize);
            strncpy(narg.mControllerNames[1], "Camera", nn::hid::ControllerSupportArg::ControllerNameSize);

            narg.mUsingControllerNames = true;
            narg.mMinPlayerCount = 2;
            narg.mMaxPlayerCount = 2;
            narg.mSingleMode = false;

            return Orig(narg);
        }
    };

    void* DoMalloc(size_t size, uintptr_t heap, int alignment) {
        return malloc(size);
    }

    // void Draw(const draw::DrawCtx& ctx) {
    //     /* Only draw if we need to show info. */
    //     if(!s_ShowDebugInfo)
    //         return;
    // 
    //     TextWriter_printf(ctx.m_TextWriter, "Camera speed: %f\n", GetCamOpSpeed());
    // 
    //     /* We need this to draw any more info. */
    //     if(s_DebugInfoStrs == NULL) {
    //         return;
    //     }
    // 
    //     /* Print all the debug strings. */
    //     for(int i = 0; i < s_DebugInfoStrs->size(); i++) {
    //         auto& str = s_DebugInfoStrs->mBuffer[i];
    //         TextWriter_printf(ctx.m_TextWriter, "%s\n", str.cstr());
    //     }
    // 
    //     /* Need a debug camera for the rest. */
    //     if(s_DbgCamera == NULL) {
    //         return;
    //     }
    // 
    //     auto drawer = ctx.OpenDrawer();
    // 
    //     auto dist = (s_DbgCamera->getAt() - s_DbgCamera->getPos()).length();
    //     drawer.DrawAxis(s_DbgCamera->getAt(), dist / 10);
    // }

    void Install() {
        namespace patch = exl::patch;
        namespace inst = exl::armv8::inst;
        namespace reg = exl::armv8::reg;
        
        SystemTaskCalcFrame::InstallAtOffset(impl::SystemTask_FuncCalcFrame);
        LayerCalc::InstallAtOffset(impl::Layer_FuncCalc);
        ShowControllerSupport::InstallAtFuncPtr(nn::hid::ShowControllerSupport);

        /* Enable allocating agl DebugInfo.*/
        patch::CodePatcher p(0x009BC214);
        p.WriteInst(inst::Nop());

        /* Force usage of malloc to allocate agl DebugInfo. */
        p.Seek(0x009BC224);
        p.BranchLinkInst((void*)DoMalloc);
        p.Seek(0x009BC248);
        p.BranchLinkInst((void*)DoMalloc);

        /* Ignore more than one controller being connected. */
        //p.Seek(0x015D2C30);
        //p.BranchInst(0x015D2C60);

        /* Adjust second controller's index to be correct. */
        //p.Seek(0x018384CC);
        //p.WriteInst(inst::Movz(reg::W8, 1));

        // p.Seek(0x015D2DB8);
        // p.Write(0x71000ABF); /* CMP W21, #2 */

        // p.Seek(0x015D2BB8);
        // p.WriteInst(inst::Movz(reg::W21, 1));
        // p.Seek(0x015D2B0C);
        // p.WriteInst(inst::Movz(reg::W21, 1));
        // // p.Seek(0x015D2D30);
        // // p.BranchInst(0x015D2E9C);
        

        // p.Seek(0x0126E040);
        // p.WriteInst(0xEFBEADDE);

        // patch::CodePatcher p(0x000D569C);
        // p.Seek(DbgInfoHeapCheck);
        // p.WriteInst(inst::Nop());
        // /* Overwrite alloc calls for DebugInfo to our func. */
        // p.Seek(DbgInfoAlloc1);
        // p.BranchLinkInst((void*) DoMalloc);
        // p.Seek(DbgInfoAlloc2);
        // p.BranchLinkInst((void*) DoMalloc);

        // draw::Subscribe(Draw);
    }   
}