/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QObject>

namespace Mayo {

class TestGraphics : public QObject {
    Q_OBJECT
private slots:
    void Regression_bugGitHub255_test();
    void ShapeObject_unboundedFaces_test();
    void ShapeObject_unboundedFaces_test_data();
    void ShapeObject_retainedFaces_test();
    void ShapeObject_retainedFaces_test_data();
    void ShapeObject_unboundedFaceStylesAndGeometry_test();
    void ShapeObject_unboundedFaceSelection_test();
    void ShapeObject_unboundedFaceCompoundSelection_test();
    void ShapeObject_unboundedFaceCompoundSelection_test_data();
    void ShapeObject_unboundedFaceInstances_test();

    void GuiApplication_documentMappingLifecycle_test();
    void GuiApplication_documentMappingCanBeDisabled_test();
    void GuiApplication_destroyDisconnectsApplicationSignals_test();

    void GuiDocument_nodeVisibilityPropagation_test();
    void GuiDocument_entityMappingLifecycle_test();
    void GuiDocument_destroyDisconnectsDocumentSignals_test();
};

} // namespace Mayo
