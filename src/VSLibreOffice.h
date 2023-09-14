#include <string>
#include <LibreOfficeKit.hxx>

class VSLibreOffice
{
    using Path = std::string;
    using Document = std::string;
    using Format = std::string;
    enum Error
    {
        Success = 0
    };

    VSLibreOffice();

    bool isInited();
    /// @brief 
    /// @pre is not inited.
    /// @return 
    Error init(const Path& pathToLibreOffice);

    bool isOpened();
    /// @brief
    /// @pre is inited
    /// @param path 
    /// @return 
    Error open(const Path& pathToFile);
    /// @brief 
    /// @pre is opened
    /// @return 
    Path opened();
    /// @brief 
    /// @pre is opened
    /// @return
    Error close();

    /// @brief 
    /// @param path 
    /// @param format 
    /// @pre is opened
    /// @return true on success
    bool saveAs(const Path& path, const Format& format) const;
    
};