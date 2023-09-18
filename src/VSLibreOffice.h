#ifndef VS_LIBRE_OFFICE
#define VS_LIBRE_OFFICE

#include <string>
#include <memory>
#include <optional>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit.hxx>

class VSLibreOffice
{
public:
    using Path = std::string;
    class Error
    {
    public:
        Error(const std::string& message);

        const std::string& message() const;

    private:
        std::string m_message;
    };

    static const std::string fileUrlPrefix;

    VSLibreOffice() = default;

    /// @brief deinitilizes office if already initialized, and tries to initialize new instance of office.
    /// @pre is not opened.
    std::optional<Error> init(const Path& pathToLibreOffice);
    /// @brief closes document if opened and deinitilizes office if initialized.
    void deinit();
    bool isInited() const;

    /// @brief closes file if opened and tries to open new file.
    /// @pre is inited
    std::optional<Error> open(const Path& pathToFile);
    /// @brief closes document if opened.
    void close();
    bool isOpened() const;

    /// @pre is opened
    int partCount() const;
    /// @pre is opened
    /// @return current part
    int part() const;
    /// @pre is opened and part is >= 0 and <= parts count.
    void setPart(int part);

    /// @brief ARGB32 = 0xAARRGGBB.
    static constexpr int bytesPerPixel = 4;
    /// @brief Renders document part to buffer in ARGB32 format.
    /// @param buffer - Buffer must accept at least bytesPerPixel * pixelWidth * pixelHeight of type unsinged char.
    /// @pre is opened
    void renderPart(int pixelWidth, int pixelHeight, unsigned char* buffer) const;

    /// @brief Saves document in specified format,
    /// if path refers to existing file, overwrites it.
    /// @pre is opened
    /// @return true on success
    std::optional<Error> saveAs(const Path& path, const std::string& format) const;

    /// @brief Closes file if opened, deinitializes if inited.
    ~VSLibreOffice();

private:
    /// @pre is inited
    Error makeError() const;


    std::unique_ptr<lok::Office> m_office;
    std::unique_ptr<lok::Document> m_document;
};

#endif //VS_LIBRE_OFFICE
