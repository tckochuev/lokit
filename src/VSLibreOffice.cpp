#include "VSLibreOffice.h"

#include <cassert>

const std::string VSLibreOffice::fileUrlPrefix = "file:///";

VSLibreOffice::Error::Error(const std::string& message) : m_message(message) {}

const std::string& VSLibreOffice::Error::message() const {
    return m_message;
}

auto VSLibreOffice::init(const Path& pathToLibreOffice) -> std::optional<Error>
{
    assert(!isOpened());
    m_office.reset(lok::lok_cpp_init(pathToLibreOffice.c_str()));
    if (!m_office) {
        return Error("Unable to initialize office from " + pathToLibreOffice);
    }
    else {
        return std::nullopt;
    }
}

void VSLibreOffice::deinit()
{
    close();
    m_office.reset();
}

bool VSLibreOffice::isInited() const
{
    return m_office != nullptr;
}

auto VSLibreOffice::open(const Path& pathToFile) -> std::optional<Error>
{
    assert(isInited());
    assert(pathToFile.find(fileUrlPrefix, 0) != 0);
    m_document.reset(m_office->documentLoad((fileUrlPrefix + pathToFile).c_str()));
    if (!m_document) {
        return makeError();
    }
    else {
        m_document->initializeForRendering();
        return std::nullopt;
    }
}

void VSLibreOffice::close()
{
    m_document.reset();
}

bool VSLibreOffice::isOpened() const
{
    return m_document != nullptr;
}

int VSLibreOffice::partCount() const
{
    assert(isOpened());
    return m_document->getParts();
}

int VSLibreOffice::part() const
{
    assert(isOpened());
    return m_document->getPart();
}

void VSLibreOffice::setPart(int part)
{
    assert(part >= 0 && part < partCount());
    m_document->setPart(part);
}

void VSLibreOffice::renderPart(int pixelWidth, int pixelHeight, unsigned char* buffer) const
{
    assert(isOpened());
    long partWidth = 0, partHeight = 0;
    m_document->getDocumentSize(&partWidth, &partHeight);
    m_document->paintTile(buffer, pixelWidth, pixelHeight, 0, 0, partWidth, partHeight);
}

auto VSLibreOffice::saveAs(const Path& path, const std::string& format) const -> std::optional<Error>
{
    assert(isOpened());
    if (!m_document->saveAs((fileUrlPrefix + path).c_str(), format.c_str())) {
        return makeError();
    }
    else {
        return std::nullopt;
    }
}

VSLibreOffice::~VSLibreOffice() {
    deinit();
}

auto VSLibreOffice::makeError() const -> Error
{
    assert(isInited());
    char* message = m_office->getError();
    Error error(message);
    m_office->freeError(message);
    return error;
}