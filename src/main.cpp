#include <iostream>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <functional>
#include <utility>
#include <iterator>
#include <cassert>
#include <unordered_set>
#include <fstream>

//#include <boost/program_options/
#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit.hxx>
#include <QImage>

const std::string fileUrlPrefix = "file:///";
const std::string directoryPath = "C:/Users/kochuev/Documents/Bugs/215824_lokit/tests/";
const std::string testInputName =
    //"input.rtf";
    //"test.odp";
    //"test2.odp";
    "test.odt";
const std::string testOutputName = "output";
const std::string testOutputExtension = "pdf";

template<typename ForwardIterator, typename PermutationForwardIterator>
size_t permutate(ForwardIterator first, ForwardIterator last,
                 PermutationForwardIterator permutationFirst,
                 typename std::iterator_traits<PermutationForwardIterator>::difference_type permutationSize
)
{
    assert(permutationSize != 0);
    //all unique
    assert(
        std::unordered_set<std::iterator_traits<PermutationForwardIterator>::value_type>(
            permutationFirst,
            std::next(permutationFirst, permutationSize)
        ).size() == permutationSize
    );
    //all in range [0, permutationSize)
    assert(
        std::all_of(
            permutationFirst, std::next(permutationFirst, permutationSize),
            [&](const auto& value) {return value >= 0 && value < permutationSize;}
        )
    );
    auto imageOf = [&](typename std::iterator_traits<PermutationForwardIterator>::difference_type index) -> auto
    {
        assert(index >= 0 && index < permutationSize);
        return *std::next(permutationFirst, index);
    };
    struct IteratorAndPermutationStatus
    {
        ForwardIterator iterator;
        bool needsPermutation;
    };
    using Buffer = std::vector<IteratorAndPermutationStatus>;
    Buffer buffer(permutationSize);
    typename Buffer::size_type vacantIndex = 0;
    while (first != last)
    {
        buffer[vacantIndex++] = {first++, true};
        if (vacantIndex == permutationSize)
        {
            for (typename Buffer::size_type i = 0; i < buffer.size(); ++i)
            {
                auto toPermutate = i;
                while (buffer[i].needsPermutation)
                {
                    auto indexSwap = [&buffer](auto lhs, auto rhs) {
                        std::iter_swap(buffer[lhs].iterator, buffer[rhs].iterator);
                    };
                    auto image = imageOf(toPermutate);
                    indexSwap(i, image);
                    buffer[image].needsPermutation = false;
                    toPermutate = image;
                }
                assert(toPermutate == i);
            }
            vacantIndex = 0;
        }
    }
    return vacantIndex;
}

template<typename ForwardIterator, typename PermutationValueType>
size_t permutate(ForwardIterator first, ForwardIterator last,
                 const std::initializer_list<PermutationValueType>& permutation
)
{
    return permutate(first, last, permutation.begin(), permutation.size());
}

int main()
{
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
    SetConsoleOutputCP(65001);

    const char* librePath = "C:\\Program Files\\LibreOffice\\program";//getenv("LIBRE");
    if (!librePath)
    {
       std::cerr << "unable to get libre office path from environment" << std::endl;
       return 1;
    }

    std::unique_ptr<lok::Office> office(lok::lok_cpp_init(librePath));
    if (!office)
    {
       std::cerr << "unable to init libre office" << std::endl;
       return 2;
    }
    auto deleter = std::bind(&lok::Office::freeError, std::ref(*office), std::placeholders::_1);
    using UniqueErrorPtr = std::unique_ptr<char, decltype(deleter)>;

    std::unique_ptr<lok::Document> document(office->documentLoad( (fileUrlPrefix + directoryPath + testInputName).c_str() ));
    if (!document)
    {
       std::cerr << "unable to load document: " << UniqueErrorPtr(office->getError(), deleter).get() << std::endl;
       return 3;
    }

    document->initializeForRendering();
    for (int i = 0; i < document->getParts(); ++i) {
        const size_t outputWidth = 640, outputHeight = 480;
        const size_t bytesPerPixel = 4;
        document->setPart(i);
        long width, height;
        document->getDocumentSize(&width, &height);
        std::cout << "Part " << i << ":" <<
            " size=" << width << "x" << height
            << std::endl;
        std::vector<unsigned char> buffer(bytesPerPixel * outputWidth * outputHeight);
        document->paintTile(buffer.data(), outputWidth, outputHeight, 0, 0, width, height);
        //const std::string BGRAFileName = "BGRA" + std::to_string(i);
        //std::ofstream fileStream(BGRAFileName);
        //if (fileStream.fail()) {
        //    std::cerr << "unable to create file " + BGRAFileName << std::endl;
        //}
        //std::copy(buffer.cbegin(), buffer.cend(), std::ostream_iterator<decltype(buffer)::value_type>(fileStream));
        /*const std::initializer_list<int> BGRAToARGBPermutation = {3, 2, 1, 0};
        auto notPermutedCount = permutate(buffer.begin(), buffer.end(), BGRAToARGBPermutation);
        assert(notPermutedCount == 0);
        QImage image(buffer.data(), outputWidth, outputHeight, QImage::Format::Format_ARGB32);
        image.detach();
        if (!image.save(QString::fromStdString(std::to_string(i)), "PNG")) {
            std::cerr << "unable to save image " << i << std::endl;
        }*/
    }

    if (!document->saveAs((fileUrlPrefix + directoryPath + testOutputName + "." + testOutputExtension).c_str(), testOutputExtension.c_str()))
    {
       std::cerr << "unable to save document: " << UniqueErrorPtr(office->getError(), deleter).get() << std::endl;
       return 4;
    }

    return 0;
}
