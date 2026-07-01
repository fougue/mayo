/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_single_format_factory.h"

#include <TopoDS_Shape.hxx>
#include <string>

namespace Mayo::IO {

// Reader for DXF file format
class DxfReader : public Reader {
public:
    DxfReader();
    ~DxfReader();

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

    struct Parameters {
        bool importAnnotations = true;
        bool groupLayers = true;
        std::string fontNameForTextObjects = "Arial";
        // TODO
        // Add syncAttribs option? If ON the reader creates missing ATTRIBs from ATTDEF
        // Or mode-like:
        //   * "strict"   -> no creation of missing ATTRIBs
        //   * "sync"     -> creates missing ATTRIBs from ATTDEF
        //   * "diagnose" -> no creation but lists "incomplete" INSERTs regarding ATTDEF
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Scans the input string and replaces TEXT control codes of the form %%x and %%nnn with their
    // corresponding Unicode characters or formatting effects
    // Replacements:
    //   %%d -> °
    //   %%c -> Ø
    //   %%p -> ±
    //   %%% -> %
    //   %%o,%%u,%%k ->
    //   %%nnn -> ?
    static void replaceTextControlCodes(std::string* str);

    // Converts a raw DXF MTEXT content string into plain text representation by stripping MTEXT
    // formatting codes and interpreting special sequences.
    // This function implements a partial MTEXT parser suitable for reading annotation text exported
    // in DXF files.
    // All MTEXT constructs that affect style, layout, or formatting but not the textual content are
    // removed or normalized.
    // Unicode escape sequences(\U+nnnn) and TEXT control codes(%%) are converted to their UTF‑8
    // equivalents.
    static std::string getPlainMText(std::string_view strMText);

private:
    // Transfer DXF objects by creating a Document root(entity) per DXF layer
    // In the resulting model tree each Document entity is actually a DXF layer
    TDF_LabelSequence transferByGroupLayers(DocumentPtr doc, TaskProgress* progress);

    // Transfer DXF objects without considering layers
    // This creates a Document root(entity) per DXF object
    TDF_LabelSequence transferBySingleEntities(DocumentPtr doc, TaskProgress* progress);

    class Properties;
    class ReaderImpl;

    Parameters m_params;
    std::unique_ptr<ReaderImpl> m_impl;
};

// Provides factory to create DxfReader objects
class DxfFactoryReader : public SingleFormatFactoryReader<Format_DXF, DxfReader> {};

} // namespace Mayo::IO
