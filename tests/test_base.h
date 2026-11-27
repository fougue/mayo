/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

class TestBase : public QObject {
    Q_OBJECT

private slots:
    // Misc
    void Enumeration_test();
    void FilePath_test();
    void MetaEnum_test();
    void OccHandle_test();
    void Quantity_test();
    void TextId_test();

    // Application
    void Application_documentLifecycle_test();
    void Application_entityLifecycle_test();
    void Application_destroyMultipleEntities_test();
    void Application_openDocument_test();
    void Application_findDocumentByIdentifier_test();
    void Application_findDocumentByLocation_test();
    void Application_findIndexOfDocument_test();
    void Application_setAutoExpandCompoundToAssembly_test();
    void Application_DocumentIterator_empty_test();
    void Application_DocumentIterator_test();

    // ApplicationItem
    void ApplicationItem_default_test();
    void ApplicationItem_document_test();
    void ApplicationItem_documentTreeNode_test();
    void ApplicationItem_equality_test();

    // ApplicationItemSelectionModel
    void ApplicationItemSelectionModel_selectedItems_test();
    void ApplicationItemSelectionModel_isSelected_test();
    void ApplicationItemSelectionModel_addItem_test();
    void ApplicationItemSelectionModel_addItems_test();
    void ApplicationItemSelectionModel_removeItem_test();
    void ApplicationItemSelectionModel_removeItems_test();
    void ApplicationItemSelectionModel_clear_test();
    void ApplicationItemSelectionModel_clearSignal_test();

    // BRepUtils
    void BRepUtils_makeEmptyCompound_test();
    void BRepUtils_addShape_nullArguments_test();
    void BRepUtils_addShape_nullTarget_test();
    void BRepUtils_addShape_existingTarget_test();
    void BRepUtils_makeEdge_test();
    void BRepUtils_makeFace_test();
    void BRepUtils_moreComplex_test();
    void BRepUtils_hashCode_test();
    void BRepUtils_shapeStringSerialization_test();
    void BRepUtils_forEachSubShape_test();
    void BRepUtils_forEachSubShapeExplorer_test();
    void BRepUtils_forEachSubFace_test();
    void BRepUtils_anySubShape_nullShape_test();
    void BRepUtils_anySubShape_emptyCompound_test();
    void BRepUtils_anySubShape_noMatchingShape_test();
    void BRepUtils_anySubShape_singleMatch_test();
    void BRepUtils_anySubShape_multipleShapes_test();
    void BRepUtils_anySubShape_stopOnFirstMatch_test();
    void BRepUtils_anySubShape_shapeType_test();
    void BRepUtils_computeMesh_test();
    void BRepUtils_computeMesh_withoutEdgesOrFaces_test();

    // CafUtils
    void CafUtils_labelTag_test();
    void CafUtils_labelAttrStdName_test();
    void CafUtils_isNullOrEmpty_test();
    void CafUtils_getNamedDataKeys_test();

    // CppUtils
    void CppUtils_indexInSpan_test();
    void CppUtils_toggle_test();

    // Document
    void Document_refCount_test();
    void Document_name_test();
    void Document_setName_test();
    void Document_isEntity_test();
    void Document_rebuildModelTree_test();
    void Document_rebuildModelTree_assembly_test();
    void Document_newEntityLabel_test();
    void Document_destroyEntity_test();

    // LibTask
    void LibTask_runAndTrackProgress_test();
    void LibTask_runJobException_test();
    void LibTask_taskCompleted_test();
    void LibTask_taskAborted_test();
    void LibTask_taskFailed_test();
    void LibTask_TaskProgress_isRoot_test();
    void LibTask_TaskManager_destroy_test();
    void LibTask_TaskManager_title_test();
    void LibTask_TaskManager_setTitle_test();
    void LibTask_TaskManager_cleanGarbage_test();

    // LibTree
    void LibTree_test();
    void LibTree_nodeRoot_test();
    void LibTree_removeRoot_test();

    // MathUtils
    void MathUtils_intRound_test();
    void MathUtils_intRound_test_data();
    void MathUtils_mappedValue_test();
    void MathUtils_mappedValue_test_data();

    // GeomUtils
    void GeomUtils_normalizedU_test();
    void GeomUtils_normalizedU_test_data();
    void GeomUtils_d0_test();
    void GeomUtils_d0_test_data();
    void GeomUtils_d1_test();
    void GeomUtils_d1_test_data();
    void GeomUtils_d0d1_test();
    void GeomUtils_d0d1_test_data();
    void GeomUtils_makeTranslation_test();
    void GeomUtils_makeRotation_test();
    void GeomUtils_makeMirror_test();
    void GeomUtils_equalPoint_test();
    void GeomUtils_equalVector_test();
    void GeomUtils_isNull_test();
    void GeomUtils_isNull_test_data();

    // MessageCollecter
    void MessageCollecter_ignoreSingleMessageType_test();
    void MessageCollecter_ignoreSingleMessageType_test_data();
    void MessageCollecter_only_test();
    void MessageCollecter_only_test_data();
    void MessageCollecter_asStringFiltered_test();
    void MessageCollecter_messageCount_test();

    // PropertyValueConversion
    void PropertyValueConversionVariant_ctor_test();
    void PropertyValueConversionVariant_ctor_test_data();
    void PropertyValueConversionVariant_toInt_test();
    void PropertyValueConversionVariant_toInt_test_data();
    void PropertyValueConversionVariant_toString_test();
    void PropertyValueConversionVariant_toString_test_data();
    void PropertyValueConversionVariant_toDouble_test();
    void PropertyValueConversionVariant_toDouble_test_data();

    void PropertyValueConversion_test();
    void PropertyValueConversion_test_data();
    void PropertyValueConversion_invalidCoordinateString_test();
    void PropertyValueConversion_invalidCoordinateString_test_data();
    void PropertyValueConversion_bugGitHub219_test();

    void PropertyQuantityValueConversion_test();
    void PropertyQuantityValueConversion_test_data();

    // MeshUtils
    void MeshUtils_test();
    void MeshUtils_test_data();
    void MeshUtils_orientation_test();
    void MeshUtils_orientation_test_data();

    // Settings
    void Settings_test();

    // StringCache
    void StringCache_addFirstTimeThenCachedReturnsSameViewAndFlag_test();
    void StringCache_addTwoSmallStringsContiguous_test();
    void StringCache_addWhenRemainingCapacityInsufficientCreatesNewPool_test();
    void StringCache_addWithNullAlreadyCachedPtrIsOK_test();
    void StringCache_clearThenReAddReturnsNewViewAndNotCached_test();
    void StringCache_viewRemainsValidAfterSubsequentAddsWithinCapacity_test();
    void StringCache_longStringLargerThanPoolWorksAndNextInsertsOK_test();
    void StringCache_stressFuzzRandomStringsDedupAndClear_test();

    // StringConv
    void StringConv_DoubleToString_test();
    void StringConv_test();

    // ThreadMessengerChannel
    void ThreadMessengerChannel_addGlobalOccPrinterReturnsNonNullPrinter_test();
    void ThreadMessengerChannel_addGlobalOccPrinterIsIdempotent_test();
    void ThreadMessengerChannel_addGlobalOccPrinter_registersExactlyOnceWithDefaultMessenger_test();
    void ThreadMessengerChannel_addGlobalOccPrinter_concurrentFirstCallsInstallExactlyOnce_test();
    void ThreadMessengerChannel_addGlobalOccPrinter_integrationForwardsOcctMessagesToScopedMessenger_test();
    void ThreadMessengerChannel_emitMessageWithoutActiveScopeIsDiscardedSilently_test();
    void ThreadMessengerChannel_emitMessageWithActiveScopeIsForwarded_test();
    void ThreadMessengerChannelScope_restoresPreviousMessengerOnDestruction_test();
    void ThreadMessengerChannelScope_supportsNestedReentrance_test();
    void ThreadMessengerChannelScope_boundToNullptrDiscardsMessages_test();
    void ThreadMessengerChannelScope_isIsolatedPerThread_test();

    // TKernelUtils
    void TKernelUtils_colorToRgb8_test();
    void TKernelUtils_colorToRgb8_test_data();
    void TKernelUtils_colorToHex_test();
    void TKernelUtils_colorToHex_test_data();
    void TKernelUtils_colorFromHex_test();
    void TKernelUtils_colorFromHex_test_data();

    // UnitSystem
    void UnitSystem_test();
    void UnitSystem_test_data();

    // XCaf
    void XCaf_isNull_test();
    void XCaf_topLevelFreeShapes_test();
    void XCaf_shapeSubs_test();
    void XCaf_shapeColor_test();
    void XCaf_validationProperties_test();
    void XCaf_diffTopLevelFreeShapes_test();
    void XCaf_shapeMaterial_test();
    void XCaf_userDefinedAttributes_test();
};

} // namespace Mayo
