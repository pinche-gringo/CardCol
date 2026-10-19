# Base image for the CI-jobs of CardCol (see .gitlab-ci.yml)
#
# Changing this file (e.g. YGP_REF to use a newer libYGP) rebuilds the image on
# the next push
FROM fedora:45

# Version of libYGP/libXGP to build the image with (tag or commit; prefer those
# over branches, as a changed branch doesn't rebuild the image)
ARG YGP_REF=134173bb2615b7c931bf268c393cc6a1064c93ef
# Repository of libYGP (passed by the build_image job)
ARG YGP_REPO

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc gcc-c++ meson ninja-build git gettext gettext-devel perl \
        boost-devel gtkmm4.0-devel librsvg2-devel ImageMagick \
        aisleriot \
        cppcheck clang-tools-extra curl \
    && dnf clean all

# Build and install libYGP/libXGP; the job token to clone it is passed as secret,
# so it doesn't end up in the image
RUN --mount=type=secret,id=job_token \
    AUTH=$(printf 'gitlab-ci-token:%s' "$(cat /run/secrets/job_token)" | base64 -w 0) \
    && git init -q /tmp/ygp && cd /tmp/ygp \
    && git -c "http.extraHeader=Authorization: Basic $AUTH" fetch -q --depth 1 "$YGP_REPO" "$YGP_REF" \
    && git checkout -q FETCH_HEAD \
    && meson setup build --prefix=/usr --buildtype=release -Dxgp=enabled \
    && meson install -C build \
    && ldconfig \
    && cd / && rm -rf /tmp/ygp
