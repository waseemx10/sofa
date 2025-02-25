/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_VISUALMODEL_INTERACTIVECAMERA_H
#define SOFA_COMPONENT_VISUALMODEL_INTERACTIVECAMERA_H
#include "config.h"

#include <SofaBaseVisual/BaseCamera.h>
#include <sofa/helper/gl/Trackball.h>
#include <sofa/core/objectmodel/KeypressedEvent.h>
#include <sofa/core/objectmodel/KeyreleasedEvent.h>
#include <sofa/core/objectmodel/MouseEvent.h>

namespace sofa
{

namespace component
{

namespace visualmodel
{

class SOFA_BASE_VISUAL_API InteractiveCamera : public BaseCamera
{
public:
    SOFA_CLASS(InteractiveCamera, BaseCamera);

    enum  { TRACKBALL_MODE, PAN_MODE, ZOOM_MODE, WHEEL_ZOOM_MODE, NONE_MODE };
    enum  { CAMERA_LOOKAT_PIVOT = 0, CAMERA_POSITION_PIVOT = 1, SCENE_CENTER_PIVOT = 2, WORLD_CENTER_PIVOT = 3};

    Data<double> p_zoomSpeed; ///< Zoom Speed
    Data<double> p_panSpeed; ///< Pan Speed
    Data<int> p_pivot; ///< Pivot (0 => Camera lookAt, 1 => Camera position, 2 => Scene center, 3 => World center

protected:
    InteractiveCamera();
    ~InteractiveCamera() override;
public:
private:
    int currentMode;
    bool isMoving;
    int lastMousePosX, lastMousePosY;
    helper::gl::Trackball currentTrackball;

    void internalUpdate() override;
protected:
    void moveCamera(int x, int y);
    void manageEvent(core::objectmodel::Event* e) override;
    void processMouseEvent(core::objectmodel::MouseEvent* me);
    void processKeyPressedEvent(core::objectmodel::KeypressedEvent* kpe);
    void processKeyReleasedEvent(core::objectmodel::KeyreleasedEvent* kre);
};

} // namespace visualmodel

} // namespace component

} // namespace sofa

#endif // SOFA_COMPONENT_VISUALMODEL_INTERACTIVECAMERA_H
