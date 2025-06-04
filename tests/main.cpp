/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_base.h"
#include "test_io.h"
#include "test_measure.h"
#include "test_app.h"

#include <QtCore/QTemporaryFile>

#include <cstring>
#include <memory>
#include <vector>

namespace {

// Helper function to return the path of some temporary file
// The file path should be unique just after function call
QString getTemporaryFilePath()
{
    QTemporaryFile file;
    if (file.open())
        return file.fileName();

    return {};
}

// Helper struct to hold the output filepath and format specified in command line
struct OutputFile {
    QString path;
    QString format;
};

// Retrieve the filename and format from string 'optionCmdLine'
// The 'optionCmdLine' argument should be the option specified in command line just after '-o'
// For example:
//     $> test-mayo -o filename,format
//     $> test-mayo -o filename
OutputFile parseOutputFile(const QString& optionCmdLine)
{
    OutputFile result;
    const int posComma = optionCmdLine.lastIndexOf(',');
    if (posComma != -1) {
        result.path = optionCmdLine.left(posComma);
        result.format = optionCmdLine.right(optionCmdLine.size() - posComma);
    }
    else {
        result.path = optionCmdLine;
    }

    return result;
}

} // namespace

int main(int argc, char* argv[])
{
    // Preprocess command-line arguments
    QStringList args;
    QString* ptrArgOutputFileName = nullptr;
    for (int i = 0; i < argc; ++i) {
        args.push_back(QString::fromUtf8(argv[i]));
        // Keep track of the output filename argument(specified after "-o" option)
        if (i > 0 && std::strcmp(argv[i-1], "-o") == 0)
            ptrArgOutputFileName = &args.back();
    }

    // Retrieve the output filename and format(separated by comma, eg "filename,format")
    const QString argOutputFileName = ptrArgOutputFileName ? *ptrArgOutputFileName : QString{};
    const OutputFile argOutputFile = parseOutputFile(argOutputFileName);

    // Declare unit tests to be checked
    std::vector<std::unique_ptr<QObject>> vecTest;
    vecTest.emplace_back(new Mayo::TestBase);
    vecTest.emplace_back(new Mayo::TestIO);
    vecTest.emplace_back(new Mayo::TestMeasure);
    vecTest.emplace_back(new Mayo::TestApp);

    // Execute unit tests
    //     As QText::qExec() is called for each test object, it would overwrite any output file
    //     specified with "-o filename,format"
    //     This is solved by substituing output file argument with a temporary file whose contents
    //     is appended to the target output file
    int retcode = 0;
    for (const std::unique_ptr<QObject>& test : vecTest) {
        // Replace the output file argument with a temporary file path
        const QString outputTestFileName = ptrArgOutputFileName ? getTemporaryFilePath() : QString{};
        if (ptrArgOutputFileName)
            *ptrArgOutputFileName = outputTestFileName + argOutputFile.format;

        // Execute test
        retcode += QTest::qExec(test.get(), args);

        // Append the temporary file to the target output file
        if (ptrArgOutputFileName) {
            const bool isFirstTest = &test == &vecTest.front();
            QFile outputTestFile(outputTestFileName);
            QFile outputFile(argOutputFile.path);
            outputTestFile.open(QIODevice::ReadOnly);
            outputFile.open(QIODevice::WriteOnly | (isFirstTest ? QIODevice::NotOpen : QIODevice::Append));
            outputFile.write(outputTestFile.readAll());
        }
    }

    return retcode;
}
