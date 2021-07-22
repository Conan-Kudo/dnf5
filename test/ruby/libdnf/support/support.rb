# Copyright (C) 2020-2021 Red Hat, Inc.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


require 'test/unit'
require 'tmpdir'
include Test::Unit::Assertions

require 'libdnf/base'
require 'libdnf/rpm'


PROJECT_BINARY_DIR = ENV["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = ENV["PROJECT_SOURCE_DIR"]


class LibdnfTestCase < Test::Unit::TestCase

    def setup()
        @base = Base::Base.new()

        @cachedir = Dir.mktmpdir("libdnf-test-ruby-")
        @base.get_config().cachedir().set(Conf::Option::Priority_RUNTIME, @cachedir)

        @repo_sack = @base.get_repo_sack()
        @package_sack = @base.get_rpm_package_sack()
    end

    def teardown()
        FileUtils.remove_entry(@cachedir)
    end

    # Add (load) a repo from `repo_path`.
    # It's also a shared code for add_repo_repomd() and add_repo_rpm().
    def _add_repo(repoid, repo_path)
        repo = @repo_sack.new_repo(repoid)

        # set the repo baseurl
        repo.get_config().baseurl().set(Conf::Option::Priority_RUNTIME, "file://" + repo_path)

        # load repository into Rpm::Repo
        repo.load()

        # load repo content into Rpm::PackageSack
        @package_sack.load_repo(repo.get(),
            Rpm::PackageSack::LoadRepoFlags_USE_FILELISTS |
            Rpm::PackageSack::LoadRepoFlags_USE_OTHER |
            Rpm::PackageSack::LoadRepoFlags_USE_PRESTO |
            Rpm::PackageSack::LoadRepoFlags_USE_UPDATEINFO
        )
    end

    # Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    def add_repo_repomd(repoid)
        repo_path = File.join(PROJECT_SOURCE_DIR, "test/data/repos-repomd", repoid)
        _add_repo(repoid, repo_path)
    end

    # Add (load) a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    def add_repo_rpm(repoid)
        repo_path = File.join(PROJECT_BINARY_DIR, "test/data/repos-rpm", repoid)
        _add_repo(repoid, repo_path)
    end

    # Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    def add_repo_solv(repoid)
        repo_path = File.join(PROJECT_SOURCE_DIR, "test/data/repos-solv", repoid + ".repo")
        @repo_sack.new_repo_from_libsolv_testcase(repoid, repo_path)
    end

end