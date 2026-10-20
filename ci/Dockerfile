# Base image for the CI-jobs of CardCol (see .gitlab-ci.yml)
#
# Changing this file (e.g. YGP_VERSION to use a newer libYGP) rebuilds the image
# on the next push
FROM fedora:45

# Version of libYGP/libXGP to build the image with; its RPMs are taken from the
# package registry of libYGP (uploaded by the release-pipeline of libYGP)
ARG YGP_VERSION=0.9.61
ARG YGP_RELEASE=1
# URL of the generic package libYGP in the package registry (passed by the build_image job)
ARG YGP_PACKAGES

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc gcc-c++ meson ninja-build git gettext gettext-devel perl rpm-build \
        boost-devel gtkmm4.0-devel librsvg2-devel ImageMagick \
        aisleriot desktop-file-utils \
        cppcheck clang-tools-extra curl \
    && dnf clean all

# Download and install the RPMs of libYGP/libXGP; the job token to access the
# registry is passed as secret, so it doesn't end up in the image
RUN --mount=type=secret,id=job_token \
    SUFFIX="${YGP_VERSION}-${YGP_RELEASE}$(rpm --eval '%{dist}').$(rpm --eval '%{_arch}').rpm" \
    && mkdir /tmp/ygp && cd /tmp/ygp \
    && for pkg in libYGP libYGP-devel libXGP libXGP-devel; do \
           curl --fail --silent --show-error --location \
                --header "JOB-TOKEN: $(cat /run/secrets/job_token)" \
                --output "$pkg-$SUFFIX" "$YGP_PACKAGES/$YGP_VERSION/$pkg-$SUFFIX" || exit 1; \
       done \
    && dnf install -y --setopt=install_weak_deps=False ./*.rpm \
    && dnf clean all \
    && cd / && rm -rf /tmp/ygp
