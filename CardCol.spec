# RPM spec file for CardCol
#
# The version is passed from outside (it's maintained in meson.build only):
#   rpmbuild -ba --define "cardcol_version 0.9.71" CardCol.spec
# with CardCol-<version>.tar.xz (from "meson dist") in the SOURCES directory.
#
# The carddecks of AisleRiot are used (KDE-carddecks and cardpics aren't packaged).

%{!?cardcol_version: %{error:Define cardcol_version (e.g. rpmbuild --define "cardcol_version 0.9.71")}}

%global app_id io.github.pinche-gringo.CardCol
%global carddir %{_datadir}/aisleriot/cards

Name:           CardCol
Version:        %{cardcol_version}
Release:        1%{?dist}
Summary:        Collection of multi-player card games
License:        GPL-2.0-or-later
URL:            https://github.com/pinche-gringo/CardCol
Source0:        %{name}-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  gettext
BuildRequires:  perl
BuildRequires:  ImageMagick
BuildRequires:  desktop-file-utils
BuildRequires:  boost-devel
BuildRequires:  gtkmm4.0-devel
BuildRequires:  librsvg2-devel
BuildRequires:  libYGP-devel
BuildRequires:  libXGP-devel
BuildRequires:  aisleriot

Requires:       aisleriot
Requires:       hicolor-icon-theme

%description
CardCol is a collection of multi-player card games (Røvhult, Twopart, Hearts,
Buraco, Machiavelli, Sgt. Mayor and Jabberwocky) played against computer
players.

%package        devel
Summary:        Header files of the card library of CardCol
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libYGP-devel
Requires:       libXGP-devel

%description    devel
Header files of the library CardCol uses to display cards and to implement
card games.

%prep
%autosetup

%build
%meson -Dnls=enabled -Drsvg=enabled -Dtests=enabled \
       -Dgnome-carddir=%{carddir} -Dkde-carddir=no -Dcardpics-dir=no
%meson_build

%install
%meson_install
install -m 644 -t %{buildroot}%{_docdir}/%{name} AUTHORS NEWS README
%find_lang %{name}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{app_id}.desktop
%meson_test

%files -f %{name}.lang
%license COPYING
%{_docdir}/%{name}/
%{_bindir}/CardCol
%{_libdir}/libCard-*.so
%{_datadir}/applications/%{app_id}.desktop
%{_datadir}/icons/hicolor/*/apps/CardCol.png

%files devel
%{_includedir}/card/

%changelog
