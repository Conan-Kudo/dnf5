/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "libdnf/common/exception.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "package_sack_impl.hpp"
#include "repo/repo_impl.hpp"
#include "solv/id_queue.hpp"
#include "solv/solv_map.hpp"
#include "utils/temp.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
#include <solv/solver.h>
#include <solv/testcase.h>
}

#include <fmt/format.h>

#include <filesystem>


using LibsolvRepo = Repo;

namespace libdnf::rpm {


void PackageSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    base->get_repo_sack()->internalize_repos();

    auto & pool = get_pool(base);
    libdnf::solv::IdQueue addedfileprovides;
    libdnf::solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(*pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());

    if (base->get_repo_sack()->has_system_repo() && !addedfileprovides_inst.empty()) {
        base->get_repo_sack()->get_system_repo()->p_impl->solv_repo.rewrite_repo(addedfileprovides_inst);
    }

    if (!addedfileprovides.empty()) {
        auto rq = repo::RepoQuery(base);
        for (auto & repo : rq.get_data()) {
            repo->p_impl->solv_repo.rewrite_repo(addedfileprovides);
        }
    }

    pool_createwhatprovides(*pool);
    provides_ready = true;
}


Package PackageSack::add_cmdline_package(const std::string & fn, bool add_with_hdrid) {
    auto repo = p_impl->base->get_repo_sack()->get_cmdline_repo();
    auto new_id = repo->p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    return Package(p_impl->base, PackageId(new_id));
}


Package PackageSack::add_system_package(const std::string & fn, bool add_with_hdrid) {
    auto new_id = p_impl->base->get_repo_sack()->get_system_repo()->p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    return Package(p_impl->base, PackageId(new_id));
}

PackageSackWeakPtr PackageSack::get_weak_ptr() {
    return PackageSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr PackageSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

int PackageSack::get_nsolvables() const noexcept {
    return p_impl->get_nsolvables();
};

PackageSack::PackageSack(const BaseWeakPtr & base)
  : p_impl{new Impl(base)},
    system_state(base->get_config().installroot().get_value())
{}

PackageSack::PackageSack(libdnf::Base & base) : PackageSack(base.get_weak_ptr()) {}

PackageSack::~PackageSack() = default;

}  // namespace libdnf::rpm
