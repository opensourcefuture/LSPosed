/*
 * This file is part of LSPosed.
 *
 * LSPosed is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LSPosed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LSPosed.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2020 EdXposed Contributors
 * Copyright (C) 2021 LSPosed Contributors
 */

#pragma once

#include <string>
#include <filesystem>
#include <sys/system_properties.h>
#include <unistd.h>
#include <sys/stat.h>
#include "logging.h"

namespace lspd {
    using namespace std::literals::string_literals;

    static inline int32_t GetAndroidApiLevel() {
        char prop_value[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", prop_value);
        return atoi(prop_value);
    }

    static inline std::string GetAndroidBrand() {
        char prop_value[PROP_VALUE_MAX];
        __system_property_get("ro.product.brand", prop_value);
        return prop_value;
    }

    template<bool quite = false>
    inline bool path_exists(const std::filesystem::path &path) {
        try {
            return std::filesystem::exists(path);
        } catch (const std::filesystem::filesystem_error &e) {
            if constexpr(!quite) {
                LOGE("%s", e.what());
            }
            return false;
        }
    }

    inline void
    path_chown(const std::filesystem::path &path, uid_t uid, gid_t gid, bool recursively = false) {
        if (chown(path.c_str(), uid, gid) != 0) {
            throw std::filesystem::filesystem_error(strerror(errno), path,
                                                    {errno, std::system_category()});
        }
        if (recursively) {
            for (const auto &item : std::filesystem::recursive_directory_iterator(path)) {
                if (chown(item.path().c_str(), uid, gid) != 0) {
                    throw std::filesystem::filesystem_error(strerror(errno), item.path(),
                                                            {errno, std::system_category()});
                }
            }
        }
    }

    inline std::tuple<uid_t, gid_t> path_own(const std::filesystem::path &path) {
        struct stat sb;
        stat(path.c_str(), &sb);
        return {sb.st_uid, sb.st_gid};
    }

    inline void recursive_permissions(const std::filesystem::path &p,
                                      std::filesystem::perms prms,
                                      std::filesystem::perm_options opts = std::filesystem::perm_options::replace) {
        std::filesystem::permissions(p, prms, opts);
        if (std::filesystem::is_directory(p)) {
            for(auto &item : std::filesystem::recursive_directory_iterator(p)) {
                std::filesystem::permissions(item.path(), prms, opts);
            }
        }
    }
}
