#pragma once

#include "workspace/interfaces/IGlobalShortcutProvider.h"
#include <map>
#include <functional>
#include <memory>

// Forward declaration for ObjC wrapper
class MacShortcutProviderPrivate;

namespace ssa::workspace {

class MacShortcutProvider : public IGlobalShortcutProvider {
public:
    MacShortcutProvider();
    ~MacShortcutProvider() override;

    int registerShortcut(ShortcutKey key, ShortcutModifier modifiers, std::function<void()> callback) override;
    bool unregisterShortcut(int id) override;

private:
    std::unique_ptr<MacShortcutProviderPrivate> m_private;
};

} // namespace ssa::workspace
