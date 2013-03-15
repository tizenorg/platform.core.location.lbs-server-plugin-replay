Name:       libslp-lbs-plugin-replay
Summary:    gps-manager plugin library for replay mode
Version:    0.1.3
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(gps-manager-plugin)

%description
gps-manager plugin library for replay mode

%define DATADIR /etc/gps-manager

%prep
%setup -q 

./autogen.sh
./configure --prefix=%{_prefix}  --datadir=%{DATADIR}


%build
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{DATADIR}/replay
cp -a nmea-log/*.log %{buildroot}%{DATADIR}/replay

%post
rm -rf /usr/lib/libSLP-lbs-plugin.so
ln -sf /usr/lib/libSLP-lbs-plugin-replay.so /usr/lib/libSLP-lbs-plugin.so

%files
%manifest libslp-lbs-plugin-replay.manifest
%defattr(-,root,root,-)
%{_libdir}/libSLP-lbs-plugin-replay.so
%{_libdir}/libSLP-lbs-plugin-replay.so.*
%{DATADIR}/replay/*
