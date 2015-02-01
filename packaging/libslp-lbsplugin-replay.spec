Name:       libslp-lbsplugin-replay
Summary:    lbs-server plugin library for replay mode
Version:    0.2.0
Release:    1
Group:      System/Libraries
License:    Apache Licensc, Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(lbs-server-plugin)
BuildRequires:  pkgconfig(deviced)

%description
lbs-server plugin library for replay mode

%define DATADIR /etc/lbs-server

%prep
%setup -q

./autogen.sh
./configure --prefix=%{_prefix}  --datadir=%{DATADIR}


%build
./autogen.sh
./configure --prefix=%{_prefix}  --datadir=%{DATADIR}
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
%manifest libslp-lbsplugin-replay.manifest
%defattr(-,root,root,-)
%{_libdir}/libSLP-lbs-plugin-replay.so*
%{DATADIR}/replay/*
