# Base image for the CI-jobs of CardCol (see .gitlab-ci.yml)
#
# Changing this file rebuilds the image on the next push. libYGP/libXGP are not
# part of the image; the jobs install the version required by meson.build
FROM fedora:45

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc gcc-c++ meson ninja-build git gettext gettext-devel perl rpm-build \
        boost-devel gtkmm4.0-devel librsvg2-devel ImageMagick \
        aisleriot desktop-file-utils \
        cppcheck clang-tools-extra curl \
    && dnf clean all
