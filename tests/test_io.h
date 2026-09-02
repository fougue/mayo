/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QObject>

namespace Mayo {

namespace IO { class System; }

class TestIO : public QObject {
    Q_OBJECT
private slots:
    void DxfReader_getPlainMText_test();
    void DxfReader_getPlainMText_test_data();
    void DxfReader_lwPolylineClosedDuplicateLastVertex_test();
    void DxfReader_replaceTextControlCodes_test();
    void DxfReader_replaceTextControlCodes_test_data();

    void ImageWriter_emptyTransfer_test();
    void ImageWriter_backgroundGradientFill_test();
    void ImageWriter_backgroundGradientFill_test_data();
    void ImageWriter_writeValidPngFile_test();
    void ImageWriter_writeValidPngFile_test_data();

    void OccStaticVariablesRollback_test();
    void OccStaticVariablesRollback_test_data();

    void Regression_bugGitHub166_test();
    void Regression_bugGitHub166_test_data();
    void Regression_bugGitHub258_test();
    void Regression_bugGitHub332_test();

    void System_importInDocument_catchVrmlReaderSendFail_test();
    void System_importInDocumentReaderException_test();
    void System_probeFormatDirect_test();
    void System_probeFormat_test();
    void System_probeFormat_test_data();

    void initTestCase();
    void cleanupTestCase();

private:
    IO::System* m_ioSystem = nullptr;
};

} // namespace Mayo
