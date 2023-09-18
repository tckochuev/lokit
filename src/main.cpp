#include <iostream>
#include <functional>
#include <cassert>
#include <charconv>

#include "VSLibreOffice.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/path.hpp>

#include <QImage>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit.hxx>

//const std::string fileUrlPrefix = "file:///";
//const std::string directoryPath = "C:/Users/kochuev/Documents/Bugs/215824_lokit/tests/";
//const std::string testInputName =
//    //"input.rtf";
//    "test.odp";
//    //"test2.odp";
//    //"test.odt";
//const std::string testOutputName = "output";
//const std::string testOutputExtension = "pdf";

namespace bpo = boost::program_options;

template<typename T>
std::optional<T> tryGetOptionAs(const bpo::variables_map& options, const std::string& optionName)
{
    if (auto found = options.find(optionName); found != std::end(options)) {
        return found->second.as<T>();
    }
    else {
        return std::nullopt;
    }
}

int main(int argc, char** argv)
{
    bpo::options_description visibleOptions("Options");
    visibleOptions.add_options()
        ("help", "show help")
        ("convert-to", bpo::value<std::string>()->value_name("format"), "convert file to specified format")
        ("output-file", bpo::value<std::string>()->value_name("path"), "path to converted file")
        ("export-as-images", bpo::value<std::string>()->value_name("format"), "exports file as images")
        ("resolution", bpo::value<std::string>()->value_name("WxH")->default_value("1920x1080"), "images resolution")
        ("output-dir", bpo::value<std::string>()->value_name("path")->default_value("."), "path to exported images");

    bpo::options_description options;
    options.add_options()
        ("libre-office", bpo::value<std::string>())
        ("file", bpo::value<std::string>());

    bpo::positional_options_description positionalOptions;
    positionalOptions.add("libre-office", 1).add("file", 2);
    bpo::variables_map optionValues;

    auto getOptionAsString = [&optionValues](const std::string& name) -> std::optional<std::string> {
        return tryGetOptionAs<std::string>(optionValues, name);
    };

    constexpr int invalidArgumentErrorReturnCode = 1;
    try {
        bpo::store(bpo::command_line_parser(argc, argv).options(options.add(visibleOptions)).positional(positionalOptions).run(), optionValues);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return invalidArgumentErrorReturnCode;
    }

    if (optionValues.empty() || getOptionAsString("help")) {
        std::cout << "Usage: lokit PATH_TO_LIBRE_OFFICE PATH_TO_FILE [--options]" << std::endl;
        std::cout << visibleOptions << std::endl;
    }

    VSLibreOffice libreOffice;
    constexpr int libreOfficeErrorReturnCode = invalidArgumentErrorReturnCode + 1;
    auto tryInitLibreOfficeAndOpenFile = [&]() -> std::optional<VSLibreOffice::Error>
    {
        if (!libreOffice.isInited())
        {
            auto libreOfficePath = getOptionAsString("libre-office");
            if (!libreOfficePath) {
                return VSLibreOffice::Error("Path to libre office installation must be provided to perform conversion or exporting");
            }
            auto error = libreOffice.init(*libreOfficePath);
            if (error) {
                return error;
            }
        }
        if (!libreOffice.isOpened())
        {
            auto filePath = getOptionAsString("file");
            if (!filePath) {
                return VSLibreOffice::Error("Path to file must be provided to perform conversion or exporting");
            }
            auto error = libreOffice.open(*filePath);
            if (error) {
                return error;
            }
        }

        return std::nullopt;
    };

    if (auto convertFormat = getOptionAsString("convert-to"))
    {
        auto initError = tryInitLibreOfficeAndOpenFile();
        if (!initError)
        {
            assert(libreOffice.isInited());
            assert(libreOffice.isOpened());
            boost::filesystem::path outputPath;
            if (auto outputPathInOptions = getOptionAsString("output-file")) {
                outputPath = *std::move(outputPathInOptions);
            }
            else
            {
                assert(getOptionAsString("file"));
                outputPath = *getOptionAsString("file");
                outputPath.replace_extension(*convertFormat);
            }
            auto error = libreOffice.saveAs(outputPath.generic_string(), *convertFormat);
            if (error) {
                std::cerr << error->message() << std::endl;
            }
        }
        else {
            std::cerr << initError->message() << std::endl;
        }
    }

    if (auto exportFormat = getOptionAsString("export-as-images"))
    {
        struct Resolution {
            int width;
            int height;
        };
        //Parse resoltion settings.
        assert(getOptionAsString("resolution"));
        std::string sresolution = *getOptionAsString("resolution");
        std::optional<Resolution> resolution;
        try {
            auto separatorPos = sresolution.find("x", 0);
            if (separatorPos == decltype(sresolution)::npos) {
                throw std::invalid_argument("");
            }
            int width = std::stoi(sresolution.substr(0, separatorPos));
            int height = std::stoi(sresolution.substr(separatorPos + 1));
            if (width < 0 || height < 0) {
                throw std::out_of_range("");
            }
            resolution = Resolution{width, height};
        }
        catch (const std::invalid_argument&)
        {
            std::cerr << "Invalid resolution format." << std::endl;
        }
        catch (const std::out_of_range&)
        {
            std::cerr << "Resolution is out of possible range." << std::endl;
        }

        if (resolution) {
            auto initError = tryInitLibreOfficeAndOpenFile();
            if (!initError)
            {
                assert(libreOffice.isInited());
                assert(libreOffice.isOpened());
                assert(getOptionAsString("output-dir"));
                boost::filesystem::path outputDir(*getOptionAsString("output-dir"));
                for (int i = 0; i < libreOffice.partCount(); ++i)
                {
                    libreOffice.setPart(i);
                    assert(resolution);
                    std::vector<unsigned char> buffer; buffer.reserve(resolution->width * resolution->height * VSLibreOffice::bytesPerPixel);
                    libreOffice.renderPart(resolution->width, resolution->height, buffer.data());
                    QImage image(buffer.data(), resolution->width, resolution->height, QImage::Format::Format_ARGB32);
                    std::string outputPath = boost::filesystem::path(outputDir).append(std::to_string(i) + "." + *exportFormat).generic_string();
                    if (!image.save(QString::fromStdString(outputPath))) {
                        std::cerr << "Unable to save " << outputPath << std::endl;
                    }
                }
            }
            else {
                std::cerr << initError->message() << std::endl;
            }
        }
    }

    //if(result != QPdfDocument::NoError) {
    //   std::cerr << QMetaEnum::fromType<QPdfDocument::DocumentError>().valueToKey(result) << std::endl;
    //   return 1;
    //}

    //while(pdfDocument.status() == QPdfDocument::Loading) {}

    //for(int i = 0; i < pdfDocument.pageCount(); ++i)
    //{
    //    QImage image = pdfDocument.render(i, pdfDocument.pageSize(i).toSize());
    //    if(image.isNull()) {
    //        std::cerr << "unable to render image " << i << std::endl;
    //        continue;
    //    }
    //    auto format = image.format();
    //    if (image.format() != QImage::Format::Format_ARGB32) {
    //        image.convertToFormat(QImage::Format::Format_ARGB32);
    //    }
    //    //QImage newImage(image.size(), image.format());
    //    //newImage.fill(QColor(Qt::white).rgb());
    //    //QPainter painter(&newImage);
    //    //painter.drawImage(0, 0, image);
    //    if(!image.save(QString::fromStdString(directoryPath + std::to_string(i) + ".png"), "png")) {
    //        std::cerr << "unable to save image" << i << std::endl;
    //    }
    //}

    //return 0;
    //SetConsoleOutputCP(65001);

    //const char* librePath = "C:\\Program Files\\LibreOffice\\program";//getenv("LIBRE");
    //if (!librePath)
    //{
    //   std::cerr << "unable to get libre office path from environment" << std::endl;
    //   return 1;
    //}

    //std::unique_ptr<lok::Office> office(lok::lok_cpp_init(librePath));
    //if (!office)
    //{
    //   std::cerr << "unable to init libre office" << std::endl;
    //   return 2;
    //}
    //auto deleter = std::bind(&lok::Office::freeError, std::ref(*office), std::placeholders::_1);
    //using UniqueErrorPtr = std::unique_ptr<char, decltype(deleter)>;

    //std::unique_ptr<lok::Document> document(office->documentLoad( (fileUrlPrefix + directoryPath + testInputName).c_str() ));
    //if (!document)
    //{
    //   std::cerr << "unable to load document: " << UniqueErrorPtr(office->getError(), deleter).get() << std::endl;
    //   return 3;
    //}

    //document->initializeForRendering();
    //for (int i = 0; i < document->getParts(); ++i) {
    //    const size_t outputWidth = 640, outputHeight = 480;
    //    const size_t bytesPerPixel = 4;
    //    document->setPart(i);
    //    long width, height;
    //    document->getDocumentSize(&width, &height);
    //    std::cout << "Part " << i << ":" <<
    //        " size=" << width << "x" << height
    //        << std::endl;
    //    std::vector<unsigned char> buffer(bytesPerPixel * outputWidth * outputHeight);
    //    document->paintTile(buffer.data(), outputWidth, outputHeight, 0, 0, width, height);
    //    const std::initializer_list<int> BGRAToARGBPermutation = {3, 2, 1, 0};
    //    //auto notPermutedCount = permutate(buffer.begin(), buffer.end(), BGRAToARGBPermutation);
    //    //assert(notPermutedCount == 0);
    //    const std::string BGRAFileName = "BGRA" + std::to_string(i);
    //    std::ofstream fileStream(BGRAFileName);
    //    if (fileStream.fail()) {
    //        std::cerr << "unable to create file " + BGRAFileName << std::endl;
    //    }
    //    std::copy(buffer.cbegin(), buffer.cend(), std::ostream_iterator<decltype(buffer)::value_type>(fileStream));
    //    QImage image(buffer.data(), outputWidth, outputHeight, QImage::Format::Format_ARGB32);
    //    const std::string format = "png";
    //    if (!image.save(QString::fromStdString(std::to_string(i) + "." + format), format.c_str())) {
    //        std::cerr << "unable to save image " << i << std::endl;
    //    }
    //}

    //if (!document->saveAs((fileUrlPrefix + directoryPath + testOutputName + "." + testOutputExtension).c_str(), testOutputExtension.c_str()))
    //{
    //   std::cerr << "unable to save document: " << UniqueErrorPtr(office->getError(), deleter).get() << std::endl;
    //   return 4;
    //}

    return 0;
}
