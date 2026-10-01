#include "Explorer.hpp"

#include <algorithm>
#include <functional>
#include <system_error>

void Explorer::open(const std::filesystem::path& path) {
    root_ = std::filesystem::absolute(path);
    selected_ = 0;
    expanded_.clear();
    scan();
}

void Explorer::refresh() {
    std::filesystem::path selPath;

    if (const ExplorerEntry* entry = curr()) selPath = entry->path;

    scan();

    for (int i = 0; i < static_cast<int>(entries_.size()); i++) {
        if (entries_[i].path == selPath) {
            selected_ = i;
            break;
        }
    }
}

const std::filesystem::path& Explorer::root() const {
    return root_;
}

const std::vector<ExplorerEntry>& Explorer::entries() const {
    return entries_;
}

int Explorer::selected() const {
    return selected_;
}

void Explorer::move(int amount) {
    if (entries_.empty()) return;

    selected_ = std::clamp(selected_ + amount, 0, static_cast<int>(entries_.size()) - 1);
}

bool Explorer::tree() const {
    return tree_;
}

void Explorer::setTree(bool enabled) {
    if (tree_ == enabled) return;

    tree_ = enabled;
    selected_ = 0;
    scan();
}

void Explorer::toggle() {
    const ExplorerEntry* entry = curr();
    if (!tree_ || !entry || !entry->dir) return;

    const std::filesystem::path path = entry->path;

    if (expanded_.find(path) != expanded_.end()) {
        expanded_.erase(path);
    } else {
        expanded_.insert(path);
    }

    scan();

    for (int i = 0; i < static_cast<int>(entries_.size()); i++) {
        if (entries_[i].path == path) {
            selected_ = i;
            break;
        }
    }
}

void Explorer::up() {
    if (root_.empty()) return;

    if (tree_) {
        const ExplorerEntry* entry = curr();
        if (!entry) return;

        if (entry->dir && entry->expanded) {
            toggle();
            return;
        }

        const std::filesystem::path parent = entry->path.parent_path();

        for (int i = 0; i < static_cast<int>(entries_.size()); i++) {
            if (entries_[i].path == parent) {
                selected_ = i;
                return;
            }
        }

        return;
    }

    const auto parent = root_.parent_path();
    if (parent.empty() || parent == root_) return;

    open(parent);
}

const ExplorerEntry* Explorer::curr() const {
    if (selected_ < 0 || selected_ >= static_cast<int>(entries_.size())) return nullptr;

    return &entries_[selected_];
}

void Explorer::scan() {
    entries_.clear();

    std::error_code error;
    if (!std::filesystem::is_directory(root_, error)) return;

    std::function<void(const std::filesystem::path&, int)> add = [&](const std::filesystem::path& path, int depth) {
        std::vector<ExplorerEntry> items;

        std::error_code scanError;
        std::filesystem::directory_iterator it(path, std::filesystem::directory_options::skip_permission_denied, scanError);
        const std::filesystem::directory_iterator end;

        while (!scanError && it != end) {
            const auto& entry = *it;
            std::error_code typeError;
            const bool dir = entry.is_directory(typeError);

            if (!typeError) {
                items.push_back({entry.path(), entry.path().filename().string(), dir, depth, false}); // NOT AGAIN...
            }

            it.increment(scanError);
        }

        std::sort(items.begin(), items.end(), [](const ExplorerEntry& a, const ExplorerEntry& b) {
            if (a.dir != b.dir) return a.dir > b.dir;
            return a.name < b.name;
        });

        for (auto& item : items) {
            item.expanded = item.dir && expanded_.find(item.path) != expanded_.end();
            entries_.push_back(item);

            if (tree_ && item.expanded && depth < 32) {
                add(item.path, depth + 1);
            }
        }
    };

    add(root_, 0);

    if (entries_.empty()) {
        selected_ = 0;
    } else {
        selected_ = std::clamp(selected_, 0, static_cast<int>(entries_.size()) - 1);
    }
}