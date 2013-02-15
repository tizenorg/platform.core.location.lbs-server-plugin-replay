Name:       libslp-lbsplugin-replay
Summary:    gps-manager plugin library for replay mode
Version:    0.1.8
Release:    1
Group:      System/Libraries
License:    Apache Licensc, Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(gps-manager-plugin)
#BuildRequires:  pkgconfig(geoclue-xps-plugin)

%description
gps-manager plugin library for replay mode

%define DATADIR /etc/gps-manager

#%package -n libslp-xpsplugin-replay
#Summary: Position/Velocity server for GeoClue (XPS)
#Group:   TO_BE/FILLED_IN

#%description -n libslp-xpsplugin-replay
#GeoClue provides applications access to various geographical information sources using a D-Bus API or a C library.-
#This package provides a positioning backend for GeoClue.

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

#%post -n libslp-xpsplugin-replay
#rm -rf /usr/lib/libSLP-xps-plugin.so
#ln -sf /usr/lib/libSLP-xps-plugin-replay.so /usr/lib/libSLP-xps-plugin.so

%files
%manifest libslp-lbsplugin-replay.manifest
%defattr(-,root,root,-)
%{_libdir}/libSLP-lbs-plugin-replay.so*
%{DATADIR}/replay/*

#%files -n libslp-xpsplugin-replay
#%manifest libslp-xpsplugin-replay.manifest
#%defattr(-,root,root,-)
#%{_libdir}/libSLP-xps-plugin-replay.so*
