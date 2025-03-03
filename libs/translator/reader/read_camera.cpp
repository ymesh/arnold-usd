//
// SPDX-License-Identifier: Apache-2.0
//

// Copyright 2022 Autodesk, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "read_shader.h"

#include <ai.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <pxr/base/gf/camera.h>
#include <pxr/usd/usdGeom/camera.h>

#include <constant_strings.h>
#include <parameters_utils.h>

#include "read_camera.h"
#include "registry.h"
#include "utils.h"
//-*************************************************************************

PXR_NAMESPACE_USING_DIRECTIVE

void UsdArnoldReadCamera::Read(const UsdPrim &prim, UsdArnoldReaderContext &context)
{
    const TimeSettings &time = context.GetTimeSettings();
    UsdGeomCamera cam(prim);

    TfToken projection;
    cam.GetProjectionAttr().Get(&projection, time.frame);

    bool persp = false;
    std::string camType;
    if (projection == UsdGeomTokens->perspective) {
        persp = true;
        camType = "persp_camera";
    } else if (projection == UsdGeomTokens->orthographic) {
        camType = "ortho_camera";
    } else {
        return;
    }

    AtNode *node = context.CreateArnoldNode(camType.c_str(), prim.GetPath().GetText());
    ReadMatrix(prim, node, time, context);

    if (persp) {
        // GfCamera has the utility functions to get the field of view,
        // so we don't need to duplicate the code here
        GfCamera gfCamera = cam.GetCamera(time.frame);
        float fov = gfCamera.GetFieldOfView(GfCamera::FOVHorizontal);
        AiNodeSetFlt(node, str::fov, fov);
        float horizontalApertureOffset = gfCamera.GetHorizontalApertureOffset();
        float verticalApertureOffset = gfCamera.GetVerticalApertureOffset();
        if (horizontalApertureOffset!=0.f || verticalApertureOffset!=0.f) {
            horizontalApertureOffset = 2.f*horizontalApertureOffset/gfCamera.GetHorizontalAperture();
            verticalApertureOffset = 2.f*verticalApertureOffset/gfCamera.GetVerticalAperture();
            AiNodeSetVec2(node, str::screen_window_min, -1+horizontalApertureOffset, -1+verticalApertureOffset);
            AiNodeSetVec2(node, str::screen_window_max, 1+horizontalApertureOffset, 1+verticalApertureOffset);
        }
        
        VtValue focusDistanceValue;
        if (cam.GetFocusDistanceAttr().Get(&focusDistanceValue, time.frame)) {
            AiNodeSetFlt(node, str::focus_distance, VtValueGetFloat(focusDistanceValue));
        }

        const float fStop = gfCamera.GetFStop();
        if (GfIsClose(fStop, 0.0f, AI_EPSILON)) {
            AiNodeSetFlt(node, str::aperture_size, 0.0f); // NOTE: str::aperture_size is a vector of float
        } else {
            AiNodeSetFlt(node, str::aperture_size, GfCamera::FOCAL_LENGTH_UNIT * gfCamera.GetFocalLength() / (2.0f * fStop));
        }
    }
    GfVec2f clippingRange;
    cam.GetClippingRangeAttr().Get(&clippingRange, time.frame);
    AiNodeSetFlt(node, str::near_clip, clippingRange[0]);
    AiNodeSetFlt(node, str::far_clip, clippingRange[1]);

    VtValue shutterOpenValue;
    if (cam.GetShutterOpenAttr().Get(&shutterOpenValue, time.frame)) {
        AiNodeSetFlt(node, str::shutter_start, VtValueGetFloat(shutterOpenValue));
    }

    VtValue shutterCloseValue;
    if (cam.GetShutterCloseAttr().Get(&shutterCloseValue, time.frame)) {
        AiNodeSetFlt(node, str::shutter_end, VtValueGetFloat(shutterCloseValue));
    }

    ReadCameraShaders(prim, node, context);

    ReadArnoldParameters(prim, context, node, time, "primvars:arnold");
    ReadPrimvars(prim, node, time, context);
}
