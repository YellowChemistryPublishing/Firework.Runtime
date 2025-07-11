#include "PackageFileCore2D.h"

using namespace Firework;
using namespace Firework::PackageSystem;

ExtensibleMarkupPackageFile::ExtensibleMarkupPackageFile(std::u8string contents) : buffer(std::move(contents))
{
    this->ok = this->doc.load_buffer_inplace(this->buffer.data(), this->buffer.size() * sizeof(char8_t));
}
