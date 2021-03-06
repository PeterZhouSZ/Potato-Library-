/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _EXAMPLES_VITELOTTE_COMMON_ORTHOGRAPHIC_CAMERA_
#define _EXAMPLES_VITELOTTE_COMMON_ORTHOGRAPHIC_CAMERA_


#include <Eigen/Geometry>


class OrthographicCamera
{
public:
    typedef Eigen::AlignedBox3f ViewBox;

public:
    OrthographicCamera();

    const ViewBox& getViewBox() const;
    void setViewBox(const ViewBox& viewBox);

    Eigen::Vector2f normalizedToCamera(const Eigen::Vector2f& norm) const;
    Eigen::Vector2f cameraToNormalized(const Eigen::Vector2f& camera) const;

    void changeAspectRatio(float ratio);
    void translate(const Eigen::Vector2f& offset);
    void zoom(const Eigen::Vector2f& center, float zoom);

    void normalizedTranslate(const Eigen::Vector2f& offset);
    void normalizedZoom(const Eigen::Vector2f& center, float zoom);

    Eigen::Matrix4f projectionMatrix() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    ViewBox m_viewBox;
};


#endif
