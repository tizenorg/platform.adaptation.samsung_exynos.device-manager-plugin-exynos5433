%bcond_with x

Name:       device-manager-plugin-exynos5433
Summary:    Device manager plugin exynos5433
Version:    0.0.1
Release:    0
Group:      System/Hardware Adaptation
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.manifest
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(hwcommon)
%if %{with x}
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
%endif

%description
Device manager plugin exynos 5433


%prep
%setup -q
cp %{SOURCE1} .

%build
%if %{with x}
%define X11_SUPPORT ON
%else
%define X11_SUPPORT OFF
%endif

%cmake . -DX11_SUPPORT:BOOL=%{X11_SUPPORT}

make %{?jobs:-j%jobs}

%install
%make_install


%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%{_libdir}/hw/*.so
%manifest %{name}.manifest
%license LICENSE
