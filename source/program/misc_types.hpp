#pragma once

#include <lib.hpp>

#include <sead/prim/seadSafeString.hpp>
#include <sead/container/seadTreeNode.h>
#include <sead/math/seadMatrix.hpp>
#include <sead/math/seadBoundBox.hpp>
#include <sead/gfx/seadDrawContext.h>
#include <sead/gfx/seadColor.h>
#include <sead/gfx/seadCamera.h>
#include <sead/gfx/seadProjection.h>
#include <sead/heap/seadDisposer.h>


namespace agl::lyr {

    struct RenderInfo {
        uint field_0;
        uint field_4;
        bool field_8;
        uintptr_t mRenderBuffer;
        uintptr_t mLayer;
        int mLayerId;
        sead::Camera *mRenderCamera;
        sead::Projection *mRenderProjection;
        uintptr_t mViewport;
        bool field_40;
        uintptr_t *mDrawContext;
    };

    struct Layer {
        struct DebugInfo {
            int field_0;
            u64 field_8[(0x68 - 0x8) / sizeof(u64)];
            u64 mDbgCamera[(0xC8 - 0x68) / sizeof(u64)];
            float mDbgCamAtDist;
            float mDbgCamTwist;
            sead::Vector3f mAtOffset;
            char field_DC[0xE8 - 0xDC];
            float mNear;
            float mFar;
            float mFovyDeg;
            sead::Vector2f mOffset;
            float mAspect;
            /* ... */
        };
    };
}